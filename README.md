Yet another C++ thread pool
=========

I had to do it and write my own thread pool class. I know it's reinventing the wheel, but I wanted to play a bit with C++11 threading features. Besides, proper high level concurrency (in spirit similar to Java executors) is not part of the standard yet, so we have to use third party stuff or write our own anyway :)

Notes
---
*  Obviously it requires C++11 capable compiler. Thread pool implementation itself (threadpool/*) relies only on standard libarary and should build anywhere.
*  Tasks are submitted as std::function&lt;void()&gt;. Any extra parameters can be wrapped inside lambdas (see example). 
*  There's no support for error handling (i.e. exception in the task will crash the program instead of being marshalled to the caller)
*  There's no explicit support for return values. It's one way "fire and forget" interface.
*  Lifetime of threads is tied to lifetime of thread pool object, and number of threads is fixed at construction. No auto sizing of any kind.
*  Implementation is rather naive. I didn't do any rigorous benchmarking of overhead introduced by thread pool but it's quite lock happy, worker threads compete on the same queue, and there's new/delete call per every submitted task. It will be fine for 2-4 cores and coarse grained tasks, but that's it.

Usage
---

Creating thread pool:
```cpp
#include "threadpool/ThreadPool.h"
(...)
ThreadPool *threadPool = createThreadPool();
(...)
delete threadPool;
```
createThreadPool() takes optional parameter with number of threads to create. If it's omitted or zero is passed, std::thread::hardware_concurrency() is used (which is reasonable default in most cases). You are responsible for deleting this object. Destructor stops all threads in the pool.

To submit task simply use lambda expression:
```cpp
threadPool->submit([=]()
{
    mandelbrotSSE2(bufferInfo, mp, CHUNK_LINE_COUNT * i, CHUNK_LINE_COUNT);
});
```
It's good idea to capture scope variables by value, otherwise kittens will die.

Synchronization
---
There are two basic synchronization primitives supported:
* fence() injects synchronization point into task queue. Fence guarantees that all previously submitted task are completed prior to executing next one. fence() does not block the client thread.
* await() is even worse because it blocks the client thread, too. You'll probably want this one to actually use the results of submitted tasks

There's surely room for improvement in this area. Right now there is fair bit of scenarios where thread pool usage will not be able to reach 100%.

Example
---
Example (files in example/*) requires libpng to build. It relies on SSE2 intrinsics (emmintrin.h header), and there's no runtime checking for SSE2 support. In theory it should build on any reasonably new compiler, but you may need to modify the code.

License
----

Public Domain  
    