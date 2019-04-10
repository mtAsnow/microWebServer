#include "Thread.h"
#include <sys/prctl.h>
#include <assert.h>
using namespace std;


pid_t gettid()
{
    return static_cast<pid_t>(::syscall(SYS_gettid));
}


class ThreadData//the new thread can occupy its only RAM space.
{
    typedef Thread::ThreadFunc ThreadFunc;
public:
    ThreadData(const ThreadFunc &func, const string& name, pid_t *tid)
    :   func_(func),
        name_(name),
        tid_(tid)
    { }

    void runInThread()
    {
        *tid_ = gettid();
        tid_ = NULL;

        //CurrentThread::t_threadName = name_.empty() ? "Thread" : name_.c_str();
        prctl(PR_SET_NAME, name_.c_str());

        func_();
    }

private:
    ThreadFunc func_;
    string name_;
    pid_t* tid_;
};

void *startThread(void* obj)
{
    ThreadData* data = static_cast<ThreadData*>(obj);
    data->runInThread();
    delete data;
    return NULL;
}


Thread::Thread(const ThreadFunc &func, const string &name)
  : started_(false),
    joined_(false),
    pthreadId_(0),
    tid_(0),
    func_(func),
    name_(name)
{
    setDefaultName();
}

Thread::~Thread()
{
    if (started_ && !joined_)
        pthread_detach(pthreadId_);
}

void Thread::setDefaultName()
{
    if (name_.empty())
    {
        char buf[32];
        snprintf(buf, sizeof buf, "Thread");
        name_ = buf;
    }
}

void Thread::start()
{
    assert(!started_);
    started_ = true;
    ThreadData* data = new ThreadData(func_, name_, &tid_);
    if (pthread_create(&pthreadId_, NULL, &startThread, data))
    {
        started_ = false;
        delete data;
    }
    else
    {
        printf("INFO:started a new thread.");
        //assert(tid_ > 0);
    }
}

int Thread::join()
{
    assert(started_);
    assert(!joined_);
    joined_ = true;
    return pthread_join(pthreadId_, NULL);
}
