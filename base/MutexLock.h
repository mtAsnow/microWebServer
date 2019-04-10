#pragma once
#include "noncopyable.h"
#include <pthread.h>
#include <errno.h>
#include <cstdio>

class MutexLock: noncopyable
{
public:
    MutexLock()
    {
        pthread_mutex_init(&mutex, NULL);
    }
    ~MutexLock()
    {
        pthread_mutex_lock(&mutex);  //for robust,avoid delete a mutex when other thread is using it.
        pthread_mutex_destroy(&mutex);
    }
    void lock()
    {
        pthread_mutex_lock(&mutex);
    }
    int trylock()
    {
        return pthread_mutex_trylock(&mutex);
    }
    void unlock()
    {
        pthread_mutex_unlock(&mutex);
    }
    pthread_mutex_t *get()
    {
        return &mutex;
    }
private:
    pthread_mutex_t mutex;

private:
    friend class Condition;
};


class MutexLockGuard: noncopyable
{
public:
    explicit MutexLockGuard(MutexLock &_mutex)://for robust,explicit !!
    mutex(_mutex)
    {
        mutex.lock();
    }
    ~MutexLockGuard()
    {
        mutex.unlock();
    }
private:
    MutexLock &mutex;
};


class TryMutexLockGuard: noncopyable
{
public:
    explicit TryMutexLockGuard(MutexLock &_mutex)://for robust,explicit !!
    mutex(_mutex)
    {
        ret = mutex.trylock();
    }
    ~TryMutexLockGuard()
    {
        mutex.unlock();
    }
    bool IfLooked()
    {
        if(ret == 0)return true;
        else if(ret == EBUSY)return false;
        else 
        {
            perror("ERR:something wrong with TryMutexLock!!\n");
        }
    }
private:
    MutexLock &mutex;
    int ret;
};
