#include "httplib.h"
#include <iostream>
#include <nlohmann/json.hpp> // JSONライブラリ
#include <vector>
#include <string>
#include <mutex>
#include <sstream>

using json = nlohmann::json;
// ─────────────────────────────────────────
// データ構造
// ─────────────────────────────────────────
struct Todo {
    int         id;
    std::string title;
    bool        done;
};

// インメモリストレージ
static std::vector<Todo> g_todos;
static int               g_next_id = 1;
static std::mutex        g_mutex;

// ─────────────────────────────────────────
// ヘルパー：Todo → JSON 文字列
// ─────────────────────────────────────────
std::string todo_to_json(const Todo& t) {
    std::ostringstream oss;
    oss << "{"
        << "\"id\":"    << t.id           << ","
        << "\"title\":\"" << t.title      << "\","
        << "\"done\":"  << (t.done ? "true" : "false")
        << "}";
    return oss.str();
}

std::string todos_to_json(const std::vector<Todo>& todos) {
    std::ostringstream oss;
    oss << "[";
    for (size_t i = 0; i < todos.size(); ++i) {
        if (i > 0) oss << ",";
        oss << todo_to_json(todos[i]);
    }
    oss << "]";
    return oss.str();
}

// ─────────────────────────────────────────
// ヘルパー：JSON から値を取り出す（簡易版）
// ─────────────────────────────────────────
std::string extract_string(const std::string& json, const std::string& key) {
    // "key":"value" を探す
    std::string pattern = "\"" + key + "\":\"";
    auto pos = json.find(pattern);
    if (pos == std::string::npos) return "";
    pos += pattern.size();
    auto end = json.find('"', pos);
    if (end == std::string::npos) return "";
    return json.substr(pos, end - pos);
}

bool extract_bool(const std::string& json, const std::string& key, bool def = false) {
    std::string pattern = "\"" + key + "\":";
    auto pos = json.find(pattern);
    if (pos == std::string::npos) return def;
    pos += pattern.size();
    return json.substr(pos, 4) == "true";
}

// ─────────────────────────────────────────
// CORS ヘッダー（ブラウザからのアクセス用）
// ─────────────────────────────────────────
void set_cors(httplib::Response& res) {
    res.set_header("Access-Control-Allow-Origin",  "*");
    res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type");
}

// ─────────────────────────────────────────
// main
// ─────────────────────────────────────────
int main() {
    httplib::Server svr;

    // ── OPTIONS（プリフライト） ──────────────
    svr.Options(".*", [](const httplib::Request&, httplib::Response& res) {
        set_cors(res);
        res.status = 204;
    });

    // ── GET /todos ── 一覧取得 ───────────────
    svr.Get("/todos", [](const httplib::Request&, httplib::Response& res) {
        std::lock_guard<std::mutex> lk(g_mutex);
        set_cors(res);
        res.set_content(todos_to_json(g_todos), "application/json");
    });

    // ── POST /todos ── 新規作成 ──────────────
    svr.Post("/todos", [](const httplib::Request& req, httplib::Response& res) {
        std::lock_guard<std::mutex> lk(g_mutex);
        //set_cors(res);
        // 1. Content-Typeの確認
        if (req.get_header_value("Content-Type") != "application/json") {
            res.status = 400;
            res.set_content("Expected application/json", "text/plain");
            return;
        }        
        try{
            // 2. JSONデコード (req.body をパース)
            json j = json::parse(req.body);

            // 3. データの取り出し (例: {"name": "Gopher", "id": 123})
            std::string title = j.at("title").get<std::string>();
            std::cout << "title=" << title << "\n";

            Todo t;
            t.id = g_next_id++;
            t.title = title;
            t.done = false;
            g_todos.push_back(t);
            res.status = 201;
            res.set_content(todo_to_json(t), "application/json");

        } catch (const std::exception& e) {
            // キーが存在しない場合など
            res.status = 500;
            res.set_content("Internal Server Error", "text/plain");
        }        
        res.status = 201;
        res.set_content("OK", "application/json");
    });

    // ── PUT /todos/:id ── 更新（title / done） ─
    svr.Put(R"(/todos/(\d+))", [](const httplib::Request& req, httplib::Response& res) {
        std::lock_guard<std::mutex> lk(g_mutex);
        set_cors(res);

        int id = std::stoi(req.matches[1]);
        for (auto& t : g_todos) {
            if (t.id == id) {
                std::string title = extract_string(req.body, "title");
                if (!title.empty()) t.title = title;
                // "done" キーが存在するときだけ更新
                if (req.body.find("\"done\"") != std::string::npos)
                    t.done = extract_bool(req.body, "done");

                res.set_content(todo_to_json(t), "application/json");
                return;
            }
        }
        res.status = 404;
        res.set_content("{\"error\":\"not found\"}", "application/json");
    });

    // ── DELETE /todos/:id ── 削除 ────────────
    svr.Delete(R"(/todos/(\d+))", [](const httplib::Request& req, httplib::Response& res) {
        std::lock_guard<std::mutex> lk(g_mutex);
        set_cors(res);

        int id = std::stoi(req.matches[1]);
        auto it = std::remove_if(g_todos.begin(), g_todos.end(),
                                 [id](const Todo& t){ return t.id == id; });
        if (it == g_todos.end()) {
            res.status = 404;
            res.set_content("{\"error\":\"not found\"}", "application/json");
            return;
        }
        g_todos.erase(it, g_todos.end());
        res.set_content("{\"result\":\"deleted\"}", "application/json");
    });

    // ── 起動 ────────────────────────────────
    int port_no = 8000;
//    std::cout << "TODO Server running on http://localhost:8080\n";
    std::cout << "TODO Server running on http://localhost:8000\n";
    std::cout << "Endpoints:\n"
              << "  GET    /todos\n"
              << "  POST   /todos        body: {\"title\":\"...\"}\n"
              << "  PUT    /todos/:id    body: {\"title\":\"...\",\"done\":true}\n"
              << "  DELETE /todos/:id\n";

    svr.listen("0.0.0.0", port_no);
    return 0;
}
