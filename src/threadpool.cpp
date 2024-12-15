#include "threadpool.h"

constexpr int TASK_MAX_THRESHHOLD = INT32_MAX;
constexpr int THREAD_MAX_THRESHHOLD = 1024;
constexpr int THREAD_MAX_IDLE_TIME = 60;

ThreadPool::ThreadPool()
    : initThreadSize_(0),
      taskSize_(0),
      idlethreadSize_(0),
      taskQueMaxTreshHold_(TASK_MAX_THRESHHOLD),
      threadMaxTreshHold_(THREAD_MAX_THRESHHOLD),
      poolMode_(PoolMode::MODE_FIXED),
      isPoolRunning_(false) {}

ThreadPool::~ThreadPool()
{
    isPoolRunning_ = false;

    // 等待线程池中所有线程返回 两张状态：阻塞 & 正在执行中
    std::unique_lock<std::mutex> lock(taskQueMtx_);
    notEmpty_.notify_all();
    exitCond_.wait(lock, [&]() -> bool
                   { return threads_.empty(); });
}

void ThreadPool::setMode(const PoolMode mode)
{
    if (checkRunnintState())
        return;

    this->poolMode_ = mode;
}

void ThreadPool::setTaskQueMaxThreshHold(int threshhold)
{
    if (checkRunnintState())
        return;
    taskQueMaxTreshHold_ = threshhold;
}

void ThreadPool::setThreadMaxThreshHold(int threshhold)
{
    if (checkRunnintState())
        return;
    if (poolMode_ == PoolMode::MODE_CACHED)
    {
        threadMaxTreshHold_ = threshhold;
    }
}

void ThreadPool::start(int initThreadSize)
{
    // 修改线程池运行状态
    isPoolRunning_ = true;
    initThreadSize_ = initThreadSize;

    for (size_t i = 0; i < initThreadSize_; i++)
    {
        auto ptr = std::make_unique<Thread>(std::bind(&ThreadPool::threadFunc, this, std::placeholders::_1));
        int threadid = ptr->getThreadId();
        threads_.emplace(threadid, std::move(ptr));

        spdlog::info("[Init] Create thread");
    }

    // 启动线程池中的线程
    for (int i = 0; i < initThreadSize_; i++)
    {
        threads_[i]->start(); // 需要去执行一个线程函数
        idlethreadSize_++;    // 记录初始空闲线程的数量
    }
}

/// @brief 线程函数，线程池中的线程会调用这个函数，从这个函数中获取任务并执行
/// @param threadid 线程id
inline void ThreadPool::threadFunc(int threadid)
{
    auto lastTime = std::chrono::high_resolution_clock().now();

    // 所有任务必须执行完成，线程池才可以收回所有线程资源
    for (;;)
    {
        Task task;
        {
            // 获取锁，上锁
            std::unique_lock<std::mutex> lock(taskQueMtx_);

            // 任务队列为空，等待任务队列有任务
            while (taskQue_.size() == 0)
            {
                // 线程池要结束，回收线程资源
                if (!isPoolRunning_)
                {
                    threads_.erase(threadid);
                    --idlethreadSize_;
                    exitCond_.notify_all();
                    spdlog::info("[Thread] Thread exit: {}", threadid);
                    return;
                }

                // 根据线程池模式选择等待方式
                if (poolMode_ == PoolMode::MODE_CACHED)
                {
                    // 条件变量超时返回了
                    if (std::cv_status::timeout == notEmpty_.wait_for(lock, std::chrono::seconds(1)))
                    {
                        auto nowTime = std::chrono::high_resolution_clock().now();
                        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(nowTime - lastTime);
                        if (duration.count() >= THREAD_MAX_IDLE_TIME && threads_.size() > initThreadSize_)
                        {
                            // Cached模式，线程空闲时间超过阈值，且线程数量大于初始线程数量，则销毁线程
                            threads_.erase(threadid);
                            --idlethreadSize_;
                            exitCond_.notify_all();
                            spdlog::info("[Thread] Thread exit: {}", threadid);
                            return;
                        }
                    }
                }
                else
                {
                    // 不是Cached模式，正常等待任务队列任务
                    notEmpty_.wait(lock);
                }
            }

            // 任务队列不为空，从任务队列中获取任务
            spdlog::info("[Run] Thread");

            task = taskQue_.front();
            taskQue_.pop();
            --taskSize_;
            --idlethreadSize_;

            // 若队列不为空，通知其他线程继续取任务
            if (taskQue_.size() > 0)
            {
                notEmpty_.notify_all();
            }
            // 队列不满,通知生产者可以提交任务
            notFull_.notify_all();

            // 执行任务
            if (task != nullptr)
                task();

            // 任务执行结束，空闲线程增加
            ++idlethreadSize_;
            lastTime = std::chrono::high_resolution_clock().now();
        }
    }
}

bool ThreadPool::checkRunnintState() const { return isPoolRunning_; }

/////////////////////线程实现/////////////////////

int Thread::generateId_ = 0;

Thread::Thread(ThreadFunc func)
    : func_(func), threadId_(generateId_++) {}

Thread::~Thread() {}

void Thread::start()
{
    std::thread t(func_, threadId_);
    t.detach();
}

int Thread::getThreadId() const { return threadId_; }
