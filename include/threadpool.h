#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <vector>
#include <queue>
#include <memory>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <thread>
#include <unordered_map>
#include <future>
#include <spdlog/spdlog.h>

// 线程池支持的模式
enum class PoolMode
{
    MODE_FIXED, // 固定数量线程模式
    MODE_CACHED // 动态增长线程数量
};

// 线程类型
class Thread
{
public:
    using ThreadFunc = std::function<void(int)>;

    Thread(ThreadFunc);

    ~Thread();

    /// @brief 启动线程
    void start();

    int getThreadId() const;

private:
    ThreadFunc func_;

    static int generateId_;

    /// @brief 线程id
    int threadId_;
};



// 线程池类型
class ThreadPool
{
public:
    ThreadPool();

    ~ThreadPool();

    /// @brief 设置线程池的工作模式
    /// @param  enum类型,工作模式
    void setMode(PoolMode);

    /// @brief 设置任务队列上限阈值
    /// @param 阈值数量
    void setTaskQueMaxThreshHold(int);

    void setThreadMaxThreshHold(int);

    template<typename Func, typename... Args>
    auto submitTask(Func&& func, Args&&... args) -> std::future<decltype(func(args...))>
    {
        using RType = decltype(func(args...));
        auto task = std::make_shared<std::packaged_task<RType()>>(std::bind(std::forward<Func>(func),std::forward<Args>(args)...));
        std::future<RType> result = task->get_future();

        // 获取锁，上锁
        std::unique_lock<std::mutex> lock(taskQueMtx_);

        // 等待任务队列空余，如果1秒内没有空余，则返回false
        if (!notFull_.wait_for(lock, std::chrono::seconds(1), [&]() -> bool
        { return taskQue_.size() < taskQueMaxTreshHold_; }))
        {
            spdlog::error("task queue is full, submit task fail.");
            auto task_ = std::make_shared<std::packaged_task<RType()>>(
                    []()->RType {return RType();}
            );
            (*task)();
            return task_->get_future();
        }

        // 队列有空余，把任务放到任务队列中
        taskQue_.emplace([task]() {(*task)();});
        ++taskSize_;

        spdlog::info("[Task] Submit Task success");

        // 因为放了新任务,任务队列不为空,在notEmpty上通知
        notEmpty_.notify_all();

        // 如果是cached模式，并且任务队列满了，需要增加线程
        if (poolMode_ == PoolMode::MODE_CACHED && taskSize_ > idlethreadSize_ && threads_.size() < threadMaxTreshHold_)
        {
            // 创建线程指针
            auto ptr = std::make_unique<Thread>(std::bind(&ThreadPool::threadFunc, this, std::placeholders::_1));
            int threadid = ptr->getThreadId();
            // 将线程加入线程池，并开启线程
            threads_.emplace(threadid, std::move(ptr));
            threads_[threadid]->start();
            ++idlethreadSize_;

            spdlog::info("[Cached] Create thread");
        }
        return result;
    }

    /// @brief 开启线程池
    void start(int = std::thread::hardware_concurrency());

    ThreadPool(ThreadPool &) = delete;

    ThreadPool &operator=(const ThreadPool &) = delete;

private:
    /// @brief 线程函数
    void threadFunc(int);

    /// @brief 获取当前线程池状态
    /// @return 线程池状态
    bool checkRunnintState() const;

private:
    /// @brief 线程列表
    std::unordered_map<int, std::unique_ptr<Thread>> threads_;

    /// @brief 初始线程数量
    size_t initThreadSize_;
    /// @brief 空闲线程数量
    std::atomic_int idlethreadSize_;
    /// @brief 线程上限阈值
    size_t threadMaxTreshHold_;

    using Task = std::function<void()>;
    /// @brief 任务队列
    std::queue<Task> taskQue_;
    /// @brief 任务数量
    std::atomic_uint taskSize_;
    /// @brief 任务队列上限阈值
    size_t taskQueMaxTreshHold_;

    /// @brief 保证任务队列的线程安全
    std::mutex taskQueMtx_;
    /// @brief 表示任务队列不满
    std::condition_variable notFull_;
    /// @brief 表示任务队列不空
    std::condition_variable notEmpty_;
    /// @brief 表示线程池退出
    std::condition_variable exitCond_;

    /// @brief 当前线程池等待工作模式
    PoolMode poolMode_;
    /// @brief 线程池是否正在运行
    std::atomic_bool isPoolRunning_;
};

#endif
