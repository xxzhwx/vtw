#pragma once
#include <stdint.h>
#include <time.h>
#include <string>

namespace vtw
{

class SystemTime final
{
public:
    SystemTime(time_t t) : _t(t) {}
    ~SystemTime() = default;

    static SystemTime now();

    std::string toString();
    std::string toFormattedString();

    operator int64_t() { return (int64_t)_t; }

private:
    time_t _t;
};

static_assert(sizeof(SystemTime) == sizeof(time_t), "Keep it simple");

} // namespace vtw
