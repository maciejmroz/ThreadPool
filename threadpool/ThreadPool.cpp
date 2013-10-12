#include "ThreadPoolImpl.h"
#include <iostream>

ThreadPool *createThreadPool(unsigned int size)
{
    return new ThreadPoolImpl(size);
}

ThreadPoolImpl::ThreadPoolImpl(unsigned int size) :
shutdownInProgress(false),
nextTaskId(0),
previousSharedTask(nullptr)
{
    if(size == 0)
    {
        size = std::thread::hardware_concurrency();
    }
    unsigned int i = 0;
    for(; i < size; ++i)
    {
        threads.push_back(new std::thread([this](){workerThreadProc();}));
    }
}

ThreadPoolImpl::~ThreadPoolImpl()
{
    shutdownInProgress = true;
    unsigned int i = 0;
    for(auto threadPtr : threads)
    {
        threadPtr->join();
        delete threadPtr;
    }
    if(previousSharedTask)
    {
        delete previousSharedTask;
    }
}

void ThreadPoolImpl::submit(const std::function<void()> &task)
{
    std::unique_lock<std::mutex>        lock(taskQueueMtx);
    taskQueue.push(new Callable(nextTaskId++,task));
}

void ThreadPoolImpl::fence()
{
    std::unique_lock<std::mutex>        lock(taskQueueMtx);
    taskQueue.push(new Barrier(nextTaskId++,threads.size()));
}

void ThreadPoolImpl::await()
{
    Barrier *barrier = new Barrier(nextTaskId++,threads.size() + 1);
    {
        std::unique_lock<std::mutex>        lock(taskQueueMtx);
        taskQueue.push(barrier);
    }
    barrier->execute();
}

void ThreadPoolImpl::tryToGetNextTask(WorkerContext &ctx)
{
    std::unique_lock<std::mutex>        lock(taskQueueMtx);
    if(!taskQueue.empty())
    {
        ctx.currentTask = taskQueue.front();
        ctx.currentTaskId = ctx.currentTask->getId();
        ctx.sharedTask = ctx.currentTask->isShared();
        if(!ctx.sharedTask)
        {
            taskQueue.pop();
        }
    }
}

void ThreadPoolImpl::tryToRemoveSharedTask(WorkerContext &ctx)
{
    std::unique_lock<std::mutex>        lock(taskQueueMtx);
    if(!taskQueue.empty())
    {
        //we are competing to remove task from the queue with other threads in the pool
        //so we need to check if the task at queue front is still the task we want remove
        if(taskQueue.front()->getId() == ctx.currentTaskId)
        {
            //we can't delete the task yet - other threads may still use it
            //however at this stage we know that the previous shared task is safe to delete
            if(previousSharedTask)
            {
                delete previousSharedTask;
            }
            previousSharedTask = ctx.currentTask;
            taskQueue.pop();
        }
    }
}

void ThreadPoolImpl::processTask(WorkerContext &ctx)
{
    ctx.currentTask->execute();
    if(!ctx.sharedTask)
    {
        delete ctx.currentTask;
    }
    else
    {
        tryToRemoveSharedTask(ctx);
    }
    ctx.currentTask = nullptr;
}

void ThreadPoolImpl::workerAction()
{
    WorkerContext ctx;

    tryToGetNextTask(ctx);

    if(ctx.currentTask)
    {
        processTask(ctx);
    }
    else
    {
        //sleep is bad here - introduces extra latency when starting to pump up the task queue
        //possibly the best solution is condition variable indicating that queue is not empty!
        std::this_thread::sleep_for(std::chrono::microseconds(10));
    }
}

void ThreadPoolImpl::workerThreadProc()
{
    while(!shutdownInProgress)
    {
        workerAction();
    }
}
