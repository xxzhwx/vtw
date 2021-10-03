#pragma once
#include <mutex>
#include "base/noncopyable.h"

namespace vtw
{

class CountDownLatch final : noncopyable
{
public:
    explicit CountDownLatch(int count = 0);

    void AddCount(int val = 1);
    void Wait();
    void CountDown(int val = 1);

    int GetCount() const;

private:
    mutable std::mutex _mutex;
    std::condition_variable _condition;

    int _count;
};

} // namespace vtw
