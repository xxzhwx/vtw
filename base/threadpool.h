#pragma once
#include <functional>
#include <stdexcept>
#include <vector>
#include <thread>
#include <atomic>
#include "base/noncopyable.h"
#include "base/lockedqueue.h"

namespace vtw
{

class ThreadPool final : noncopyable
{
public:
    using Task = std::function<void()>;

    ThreadPool(size_t threadNum)
    {
        if (threadNum == 0 || threadNum > (1 << 10))
        {
            throw std::invalid_argument("invalid thread number");
        }

        for (size_t i = 0; i < threadNum; ++i)
        {
            _threads.emplace_back([this] { worker(); });
        }
    }

    ~ThreadPool()
    {
        for (auto &t : _threads)
        {
            if (t.joinable())
                t.join();
        }
    }

    void Stop() { _stop = true; }

    void AddTask(const Task& t)
    {
        if (!_stop)
        {
            _q.Put(t);
        }
    }

private:
    void worker()
    {
        static const long long waitTime = 1000; // one second

        while (!_stop)
        {
            Task t;
            if (_q.TryTake(t, waitTime))
            {
                t();
            }
        }
    }

    LockedQueue<Task> _q;
    std::vector<std::thread> _threads;
    std::atomic_bool _stop{ false };
};

} // namespace vtw
