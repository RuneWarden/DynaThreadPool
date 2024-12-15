# DynaThreadPool

此项目实现了一个基于可变参数模板的线程池，旨在提供一个高效且灵活的多线程任务处理框架。通过使用 C++ 11 中的可变参数模板，可以方便地传递任意数量的参数来执行任务。线程池还使用了 spdlog 库来提供高效的日志记录功能。

## 特性

- 基于可变参模板编程和引用折叠原理，实现线程池`submitTask`接口，支持任意任务函数和任意参数的传递
- 使用`future`类型定制`submitTask`提交任务的返回值
- 使用`map`和`queue`容器管理线程对象和任务
- 基于条件变量`condition_variable`和互斥锁`mutex`实现任务提交线程和任务执行线程间的通信机制
- 支持`fixed`和`cached`模式的线程池定制
- 集成 `spdlog` 库进行日志记录，支持调试、错误等不同日志级别
- 任务队列采用生产者-消费者模式进行处理
  
## 结构
``` bash
├─bin
|  └─test.exe
├─include
|  └─threadpool.h
├─lib
|  └─libdtp.a
├─src
|  └─threadpool.cpp
└─test
   ├─CMakeLists.txt
   └─test.cpp
```
