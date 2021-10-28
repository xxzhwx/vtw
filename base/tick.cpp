#include "base/tick.h"
#include <chrono>

namespace vtw
{

static int64_t __getTick()
{
    std::chrono::steady_clock::time_point tp = std::chrono::steady_clock::now();
    std::chrono::milliseconds tick = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch());
    return tick.count();
}

static const int64_t __startTick = __getTick();

int64_t GetTickCount()
{
    return __getTick() - __startTick;
}

} // namespace vtw
