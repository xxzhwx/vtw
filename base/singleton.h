#pragma once
#include <memory>
#include <mutex>
#include "base/noncopyable.h"

namespace vtw
{

template<typename T>
class Singleton final : noncopyable
{
public:
    Singleton() = delete;
    ~Singleton() = delete;

    static T& Instance()
    {
        static std::once_flag flag;
        std::call_once(flag, [&]() { _ptr.reset(new T); });
        return *_ptr;
    }

private:
    static std::unique_ptr<T> _ptr;
};

template<typename T>
std::unique_ptr<T> Singleton<T>::_ptr;

} // namespace vtw
