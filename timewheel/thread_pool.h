#pragma once
#include <atomic>
#include <exception>
#include <functional>
#include <thread>
#include <vector>

#include "timewheel/locked_queue.h"

using TimerFunc = std::function<void()>;
using TimerQueue = LockedQueue<TimerFunc>;

class ThreadPool final
{
public:
    ThreadPool(size_t n)
    {
        if (n == 0 || n > (1 << 10))
        {
            throw std::invalid_argument("invalid thread number");
        }

        for (size_t i = 0; i < n; ++i)
        {
            threads_.emplace_back([this] { worker_loop(); });
        }
    }

    ~ThreadPool()
    {
        for (auto &t : threads_)
        {
            if (t.joinable())
            {
                t.join();
            }
        }
    }

    // no coping
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    void Enqueue(const TimerFunc& f) { q_.Enqueue(f); }
    void Stop() { stop_ = true; }

private:
    void worker_loop()
    {
        while (!stop_)
        {
            TimerFunc f;
            if (q_.Dequeue(f))
            {
                f();
            }
        }
    }

    TimerQueue q_;
    std::vector<std::thread> threads_;
    std::atomic_bool stop_{ false };
};
