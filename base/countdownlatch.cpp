#include "base/countdownlatch.h"

namespace vtw
{

CountDownLatch::CountDownLatch(int count /* = 0 */)
    : _mutex()
    , _condition()
    , _count(count)
{
}

void CountDownLatch::AddCount(int val /* = 1 */)
{
    std::lock_guard<std::mutex> guard(_mutex);
    _count += val;
}

void CountDownLatch::Wait()
{
    std::unique_lock<std::mutex> lock(_mutex);
    _condition.wait(lock, [&]() { return _count <= 0; });
}

void CountDownLatch::CountDown(int val /* = 1 */)
{
    std::lock_guard<std::mutex> guard(_mutex);
    _count -= val;
    if (_count <= 0)
    {
        _condition.notify_all();
    }
}

int CountDownLatch::GetCount() const
{
    std::lock_guard<std::mutex> guard(_mutex);
    return _count;
}

} // namespace vtw
