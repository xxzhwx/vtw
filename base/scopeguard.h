#pragma once
#include <functional>
#include <base/noncopyable.h>

namespace vtw
{

// 用于资源释放，类似 golang 的 defer
// 直接使用 ON_SCOPE_EXIT 宏即可
class ScopeGuard : noncopyable
{
public:
    explicit ScopeGuard(std::function<void()> onExitScope)
        : _onExitScope(onExitScope)
        , _dismissed(false)
    {

    }

    ~ScopeGuard()
    {
        if (!_dismissed)
        {
            _onExitScope();
        }
    }

    void Dismiss()
    {
        _dismissed = true;
    }

private:
    std::function<void()> _onExitScope;
    bool _dismissed;
};

// use line-number as variable name
#define SCOPEGUARD_LINENAME_CAT(name, line) name##line
#define SCOPEGUARD_LINENAME(name, line) SCOPEGUARD_LINENAME_CAT(name, line)

#define ON_SCOPE_EXIT(callback) ScopeGuard SCOPEGUARD_LINENAME(EXIT, __LINE__)(callback)

} // namespace vtw
