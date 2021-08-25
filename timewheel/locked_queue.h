#pragma once
#include <deque>
#include <mutex>
#include <condition_variable>

template<typename T>
class LockedQueue
{
public:
    LockedQueue() = default;
    ~LockedQueue() = default;

    void Enqueue(const T& t)
    {
        std::lock_guard<std::mutex> lock(m_);
        q_.emplace_back(t);
        cv_.notify_one();
    }

    void Enqueue(T&& t)
    {
        std::lock_guard<std::mutex> lock(m_);
        q_.emplace_back(std::move(t));
        cv_.notify_one();
    }

    bool Dequeue(T& t)
    {
        std::unique_lock<std::mutex> lock(m_);
        // wait_for returns false if the predicate pred still evaluates to false after the rel_time timeout expired, otherwise true.
        if (cv_.wait_for(lock, std::chrono::seconds(1), [this] {return !q_.empty();})) //! wait_for 会隐式释放互斥锁上的锁，所以要用 unique_lock
        {
            t = q_.front();
            q_.pop_front();
            return true;
        }
        return false;
    }

private:
    std::deque<T> q_;
    std::mutex m_;
    std::condition_variable cv_;
};
