#pragma once
#include <algorithm>
#include <chrono>
#include <deque>
#include <vector>
#include <mutex>
#include <condition_variable>
#include "base/noncopyable.h"

namespace vtw
{

template<typename T>
class LockedQueue final : noncopyable
{
public:
    LockedQueue() = default;
    ~LockedQueue() = default;

    void Put(const T& t)
    {
        std::lock_guard<std::mutex> guard(_m);
        _q.emplace_back(t);
        _cv.notify_one();
    }

    void Put(T&& t)
    {
        std::lock_guard<std::mutex> guard(_m);
        _q.emplace_back(std::move(t)); // std::move is more specific than std:forward
        _cv.notify_one();
    }

    void Take(T& t)
    {
        std::unique_lock<std::mutex> lock(_m);
        _cv.wait(lock, [this]() { return !_q.empty(); });

        t = _q.front();
        _q.pop_front();
    }

    bool TryTake(T& t)
    {
        std::lock_guard<std::mutex> guard(_m);
        if (_q.empty())
            return false;

        t = _q.front();
        _q.pop_front();
        return true;
    }

    bool TryTake(T& t, long long waitTimeInMilliSecond)
    {
        std::unique_lock<std::mutex> lock(_m);
        // wait_for returns false if the predicate pred still evaluates to false after the rel_time timeout expired, otherwise true.
        if (_cv.wait_for(lock, std::chrono::milliseconds(waitTimeInMilliSecond), [this] { return !_q.empty(); }))
        {
            t = _q.front();
            _q.pop_front();
            return true;
        }

        return false;
    }

    bool TryTakeAll(std::vector<T>& all)
    {
        std::lock_guard<std::mutex> guard(_m);
        if (_q.empty())
            return false;

        std::move(_q.begin(), _q.end(), all.begin());
        _q.clear();
        return true;
    }

private:
    std::deque<T> _q;
    std::mutex _m;
    std::condition_variable _cv;
};

} // namespace vtw
