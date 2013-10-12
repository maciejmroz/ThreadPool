
#ifndef _THREAD_POOL_H_
#define _THREAD_POOL_H_

#include <functional>

/**
 *  Very, very simple thread pool.
 *
 *  Tasks are submitted as void() functions. Any extra parameters can be wrapped in lambdas.
 *  There's no support for error handling (i.e. exception in the task will crash the program
 *  instead of being marshalled to the caller) and no explicit support for return values.
 *  Lifetime of threads is tied to lifetime of thread pool object, and number of threads is
 *  fixed at construction.
 */
class ThreadPool {
public:
    virtual ~ThreadPool() {};

    /**
     *  Submit a task to be run. It will be executed by the first available thread
     */
    virtual void submit(const std::function<void()> &task) = 0;

    /**
     *  Inject synchronization point into task queue. Fence requires that all previously submitted tasks
     *  are completed prior to executing next one. fence() does not block the calling thread.
     */
    virtual void fence() = 0;

    /**
     *  Block until all tasks in the queue are completed.
     */
    virtual void await() = 0;
};

/**
 *  Create new instance of ThreadPool with number of threads given by size. Omitting size
 *  initializes it to std::thread::hardware_concurrency (which hopefully is a number of logical CPU cores).
 */
extern ThreadPool *createThreadPool(unsigned int size = 0);

#endif