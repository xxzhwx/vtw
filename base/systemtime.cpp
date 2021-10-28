#include "base/systemtime.h"
#include <stdio.h>
#include <chrono>

namespace vtw
{

SystemTime SystemTime::now()
{
    auto tp = std::chrono::system_clock::now();
    time_t t = std::chrono::system_clock::to_time_t(tp);
    return SystemTime(t);
}

std::string SystemTime::toString()
{
    return std::to_string(_t);
}

std::string SystemTime::toFormattedString()
{
    char buf[64] = { 0 };

    tm* tm_time = ::localtime(&_t);
    snprintf(buf, sizeof(buf), "%4d/%02d/%02d %02d:%02d:%02d",
        tm_time->tm_year + 1900,
        tm_time->tm_mon + 1,
        tm_time->tm_mday,
        tm_time->tm_hour,
        tm_time->tm_min,
        tm_time->tm_sec);

    return buf;
}

} // namespace vtw
