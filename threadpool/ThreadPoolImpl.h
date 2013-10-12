
#ifndef _THREAD_POOL_IMPL_H_
#define _THREAD_POOL_IMPL_H_

#include "ThreadPool.h"
#include <vector>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <queue>

class Task {
    unsigned long long id;
public:
    Task(unsigned long long taskId);
    virtual ~Task();

    unsigned long long getId();

    virtual void execute() = 0;
    virtual bool isShared() = 0;
};

class Callable : public Task {
    std::function<void()>       task;
public:
    Callable(unsigned long long taskId, const std::function<void()> &task_p);
    ~Callable();

    void execute();
    bool isShared();
};

class Barrier : public Task {
    std::mutex                  barrierMtx;
    std::condition_variable     barrier;
    int                         currentSize;
public:
    Barrier(unsigned long long taskId, int initialSize);
    ~Barrier();

    void execute();
    bool isShared();
};

struct WorkerContext
{
    unsigned long long currentTaskId;
    bool sharedTask;
    Task *currentTask;

    WorkerContext() :
    currentTaskId(0),
    sharedTask(false),
    currentTask(nullptr)
    {
    }
};

class ThreadPoolImpl : public ThreadPool {
    std::vector<std::thread*>       threads;
    std::mutex                      taskQueueMtx;
    std::queue<Task*>               taskQueue;
    Task                            *previousSharedTask;

    volatile bool                   shutdownInProgress;
    unsigned long long              nextTaskId;

    void tryToGetNextTask(WorkerContext &ctx);
    void tryToRemoveSharedTask(WorkerContext &ctx);
    void processTask(WorkerContext &ctx);
    void workerAction();
    void workerThreadProc();
public:
    ThreadPoolImpl(unsigned int size);
    ~ThreadPoolImpl();

    void submit(const std::function<void()> &task);
    void fence();
    void await();
};

#endif