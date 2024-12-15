#include "threadpool.h"
#include <iostream>
#include <thread>
#include <future>
#include <chrono>

int sum(int a, int b)
{
    std::this_thread::sleep_for(std::chrono::seconds(3));
    return a + b;
}

int main()
{
    ThreadPool pool;
    pool.setMode(PoolMode::MODE_CACHED);
    pool.start(2);
    std::future<int> r1 = pool.submitTask(sum, 1, 2);
    std::future<int> r2 = pool.submitTask(sum, 1, 2);
    std::future<int> r3 = pool.submitTask(sum, 1, 2);
    std::future<int> r4 = pool.submitTask(sum, 1, 2);

    spdlog::info("{}",r1.get());
    spdlog::info("{}",r2.get());
    spdlog::info("{}",r3.get());
    spdlog::info("{}",r4.get());

    return 0;
}
