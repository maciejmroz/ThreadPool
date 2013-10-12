#include "ThreadPoolImpl.h"

Task::Task(unsigned long long taskId) :
id(taskId)
{
}

Task::~Task()
{
}

unsigned long long Task::getId()
{
    return id;
}

Callable::Callable(unsigned long long taskId, const std::function<void()> &task_p) :
Task(taskId),
task(task_p)
{
}

Callable::~Callable()
{
}

void Callable::execute()
{
    task();
}

bool Callable::isShared()
{
    return false;
}

Barrier::Barrier(unsigned long long taskId, int initialSize) :
Task(taskId)
{
    std::unique_lock<std::mutex> lock(barrierMtx);
    currentSize = initialSize;
}

Barrier::~Barrier()
{
}

void Barrier::execute()
{
    {
        std::unique_lock<std::mutex> lock(barrierMtx);
        currentSize--;
        barrier.wait(lock,[this]{return currentSize<=0;});
    }
    //hmmm, notify_one() may be better choice here
    barrier.notify_all();
}

bool Barrier::isShared()
{
    return true;
}
