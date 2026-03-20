# todo_3

 Version: 0.9.1

 date    : 2026/03/20

 update :

***

C++ , sqlite database todo 

* gcc version 14.2.0 

***

### setup

* LIB

```
sudo apt-get install libsqlite3-dev
```

***
* DB_path change, main.cpp
```
std::string DB_PATH = "/home/user123/todo_3/todo.db";
```


***
* build
```
make all
```

***
* test-data

* add
```
./todo add hello
```
***
* delete , id input

```
./todo rm 1
```

***
* list
```
todo list
```

***
