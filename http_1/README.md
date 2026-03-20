# http_1

 Version: 0.9.1

 date    : 2026/03/20

 update :

***

C++ cpp-httplib , todo API server

* gcc version 14.2.0 

***

### setup

* json-LIB

```
sudo apt install nlohmann-json3-dev
```
* related

https://github.com/yhirose/cpp-httplib/blob/master/httplib.h

***
* build
```
make all
```

* start
```
./server
```

***
* test-data

* add
```
curl -X POST http://localhost:8000/todos \
     -H "Content-Type: application/json" \
     -d '{"title": "tit-21"}'
```

* DELETE
```
curl -X DELETE http://localhost:8000/todos/3
```

* list
```
curl http://localhost:8000/todos
```
***
### blog

https://zenn.dev/knaka0209/scraps/4668e69a3452d4

