#pragma once
#include <cstdint>
#include <chrono>
//#include <memory>
#include <unordered_map>

#include "timewheel/thread_pool.h"

struct Timer
{
    Timer* prev{ nullptr };
    Timer* next{ nullptr };
    // timer id
    int64_t id{ 0 };
    // expires time
    int64_t expires{ 0 };
    // > 0 circle timer
    int64_t interval{ 0 };
    // function
    TimerFunc func;

    Timer() = default;
    Timer(int64_t id, int64_t e, int64_t i, const TimerFunc& f)
        : id(id), expires(e), interval(i), func(f) {}
};

using TimerPtr = std::shared_ptr<Timer>;

// insert tail
inline void timer_emplace_back(Timer* head, Timer* n)
{
    if (!head || !n) return;
    auto tail = head->prev;
    tail->next = n;
    n->prev = tail;
    n->next = head;
    head->prev = n;
}

inline void timer_erase(Timer* n)
{
    if (!n) return;
    auto prev = n->prev;
    auto next = n->next;
    if (!prev || !next) return;
    prev->next = next;
    next->prev = prev;
}

class TimeWheel
{
public:
    // wheel slot: 8 + 4 * 6
    static constexpr int64_t WHEEL_SIZE1 = 1LL << 8;
    static constexpr int64_t WHEEL_SIZE2 = 1LL << 6;

    // wheel range
    static constexpr int64_t FIRST_RANGE = (1LL << 8) - 1;
    static constexpr int64_t SECOND_RANGE = (1LL << 14) - 1;
    static constexpr int64_t THIRD_RANGE = (1LL << 20) - 1;
    static constexpr int64_t FOURTH_RANGE = (1LL << 26) - 1;
    static constexpr int64_t FIFTH_RANGE = (1LL << 32) - 1;

    // 100ms, 10ms, 1ms
    // for high, the range is [0, 2^32) ms, about 0~49.7day
    // for medium, the range is 10 * [0, 2^32) ms, about 0~1.36year
    // for low, the range is 100 * [0, 2^32) ms, about 0~13.6year
    enum class precision : int64_t { low = 100, medium = 10, high = 1 };

    // time wheel level, 0~4
    enum
    {
        first_wheel = 0,
        second_wheel,
        third_wheel,
        fourth_wheel,
        fifth_wheel
    };

    static int64_t now_milli()
    {
        auto now = std::chrono::steady_clock::now().time_since_epoch();
        return std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
    }

    TimeWheel(std::weak_ptr<ThreadPool> tp, precision p = precision::high)
        : now_(now_milli())
        , precision_(static_cast<int64_t>(p))
        , threadPool_(tp)
    {
        // init time wheel
        for (int64_t i = 0; i <= fifth_wheel; ++i)
        {
            std::vector<TimerPtr> tmp;
            for (int64_t j = 0; j < (i==0 ? WHEEL_SIZE1 : WHEEL_SIZE2); ++j)
            {
                auto it = std::make_shared<Timer>();
                it->prev = it.get();
                it->next = it.get();
                tmp.emplace_back(std::move(it));
            }
            wheel_.emplace_back(std::move(tmp));
        }

        loopThread_ = std::thread([this] { WorkerLoop(); });
    }

    ~TimeWheel()
    {
        if (loopThread_.joinable())
        {
            loopThread_.join();
        }
    }

    // no copying
    TimeWheel(const TimeWheel&) = delete;
    TimeWheel& operator=(const TimeWheel&) = delete;

    // timeout: ms
    int64_t Add(int64_t timeout, const TimerFunc& f)
    {
        return Add(timeout, 0, f);
    }

    // interval: ms, > 0, cycle timer
    int64_t Add(int64_t timeout, int64_t interval, const TimerFunc& f)
    {
        auto adjust = [this](int64_t& time) {
            if (time <= 0)
            {
                time = 0;
            }
            else
            {
                int64_t tmp = time / precision_;
                if (tmp > FIFTH_RANGE) tmp = FIFTH_RANGE;
                time = tmp * precision_;
            }
        };

        adjust(timeout);
        adjust(interval);
        int64_t timerid = 0;

        {
            std::lock_guard<std::mutex> lock(loopMutex_);
            timerid = GenId();
            auto ptr = std::make_shared<Timer>(timerid, now_ + timeout, interval, f);
            timerMap_.emplace(timerid, ptr);
            Add(ptr.get());
        }

        return timerid;
    }

    void Erase(int64_t id)
    {
        std::lock_guard<std::mutex> lock(loopMutex_);
        auto it = timerMap_.find(id);
        if (it == timerMap_.end()) return;
        timer_erase(it->second.get());
        timerMap_.erase(it);
    }

    void Stop()
    {
        std::lock_guard<std::mutex> lock(loopMutex_);
        stop_ = true;
    }

private:
    int64_t GenId()
    {
        ++id_;
        if (id_ == 0) ++id_;
        while (timerMap_.find(id_) != timerMap_.end()) ++id_;
        return id_;
    }

    int64_t GetSlot(int64_t time, int64_t wheelLevel)
    {
        time /= precision_;
        if (wheelLevel == first_wheel)
        {
            return time & 255;
        }
        else if (wheelLevel > first_wheel && wheelLevel <= fifth_wheel)
        {
            return time >> (8 + (wheelLevel - 1) * 6) & 63;
        }
        return 0;
    }

    void Add(Timer* ptr)
    {
        int64_t duration = ptr->expires - now_;
        duration /= precision_;
        int64_t level = 0, slot = 0;

        // 0 ~ 255
        if (duration < FIRST_RANGE)
        {
            slot = GetSlot(ptr->expires, first_wheel);
            level = first_wheel;
        }
        // 255 ~ 2^14-1
        else if (duration < SECOND_RANGE)
        {
            slot = GetSlot(ptr->expires, second_wheel);
            level = second_wheel;
        }
        // 2^14 ~ 2^20-1
        else if (duration < THIRD_RANGE)
        {
            slot = GetSlot(ptr->expires, third_wheel);
            level = third_wheel;
        }
        // 2^20 ~ 2^26-1
        else if (duration < FOURTH_RANGE)
        {
            slot = GetSlot(ptr->expires, fourth_wheel);
            level = fourth_wheel;
        }
        // 2^26 ~ 2^32-1
        else if (duration < FIFTH_RANGE)
        {
            slot = GetSlot(ptr->expires, fifth_wheel);
            level = fifth_wheel;
        }
        else
        {
            return;
        }

        Timer* head = wheel_[level][slot].get();
        timer_emplace_back(head, ptr);
    }

    void Transfer(int64_t level, int64_t slot)
    {
        auto head = wheel_[level][slot].get();
        if (head->next == head) return;

        auto node = head->next;
        while (node != head)
        {
            auto tmp = node;
            node = node->next;
            Add(tmp);
        }

        head->next = head;
        head->prev = head;
    }

    void Foreach(int64_t level, int64_t slot)
    {
        auto head = wheel_[level][slot].get();
        if (head->next == head) return;

        auto list = head->next;

        while (list != head)
        {
            auto tmp = list;
            list = list->next;

            // exec timer function
            if (tmp->func)
            {
                if (auto tp = threadPool_.lock())
                {
                    tp->Enqueue(tmp->func);
                }
                else
                {
                    throw std::runtime_error("no thread runs the timer function");
                }
            }

            if (tmp->interval > 0)
            {
                tmp->expires += tmp->interval;
                Add(tmp);
            }
            else
            {
                timerMap_.erase(tmp->id);
            }
        }

        head->next = head;
        head->prev = head;
    }

    void WorkerLoop()
    {
        while (true)
        {
            auto present = std::chrono::steady_clock::now();
            auto nextTick = present + std::chrono::milliseconds(precision_);
            int64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(
                present.time_since_epoch()).count();
            int64_t slot = 0, firstSlot = 0;

            {
                std::lock_guard<std::mutex> lock(loopMutex_);
                if (stop_) break;

                while (now - now_ >= 0)
                {
                    slot = GetSlot(now_, first_wheel);
                    firstSlot = slot;
                    if (slot == 0)
                    {
                        slot = GetSlot(now_, second_wheel);
                        Transfer(second_wheel, slot);
                        if (slot == 0)
                        {
                            slot = GetSlot(now_, third_wheel);
                            Transfer(third_wheel, slot);
                            if (slot == 0)
                            {
                                slot = GetSlot(now_, fourth_wheel);
                                Transfer(fourth_wheel, slot);
                                if (slot == 0)
                                {
                                    slot = GetSlot(now_, fifth_wheel);
                                    Transfer(fifth_wheel, slot);
                                }
                            }
                        }
                    }

                    Foreach(first_wheel, firstSlot);
                    now_ += precision_;
                }
            }

            std::this_thread::sleep_until(nextTick);
        }
    }

    int64_t id_{ 0 };
    int64_t now_{ 0 };
    int64_t precision_{ 0 };
    std::weak_ptr<ThreadPool> threadPool_;
    std::unordered_map<int64_t, TimerPtr> timerMap_;
    std::vector<std::vector<TimerPtr>> wheel_;

    std::thread loopThread_;
    std::mutex loopMutex_;
    bool stop_{ false };
};
