#pragma once
#include <functional>
#include "base/noncopyable.h"
#include "net/socketdatatype.h"

namespace vtw
{
namespace net
{

class Poller;
class Channel : noncopyable
{
public:
    using EventCallback = std::function<void()>;

    Channel(Poller* poller, VTW_SocketHandle handle);
    ~Channel();

    void SetReadCallback(EventCallback cb) { _readCallback = std::move(cb); }
    void SetWriteCallback(EventCallback cb) { _writeCallback = std::move(cb); }
    void SetCloseCallback(EventCallback cb) { _closeCallback = std::move(cb); }
    void SetErrorCallback(EventCallback cb) { _errorCallback = std::move(cb); }
    Poller* GetPoller() const { return _poller; }
    VTW_SocketHandle Handle() const { return _handle; }
    int Events() const { return _events; }

    void EnableReading() { _events |= kPollIn; update(); }
    void DisableReading() { _events &= ~kPollIn; update(); }
    void EnableWriting() { _events |= kPollOut; update(); }
    void DisableWriting() { _events &= ~kPollOut; update(); }
    void DisableAll() { _events = kPollNone; update(); }
    bool IsNoneEvent() { return _events == kPollNone; }
    bool IsReading() { return _events & kPollIn; }
    bool IsWriting() { return _events & kPollOut; }

    void HandleReadEvent() { if (_readCallback != nullptr) { _readCallback(); } }
    void HandleWriteEvent() { if (_writeCallback != nullptr) { _writeCallback(); } }
    void HandleCloseEvent() { if (_closeCallback != nullptr) { _closeCallback(); } }
    void HandleErrorEvent() { if (_errorCallback != nullptr) { _errorCallback(); } }

    void Remove();

private:
    void update();

private:
    static const int kPollNone = 0;
    static const int kPollIn = 0x0001;
    static const int kPollOut = 0x0002;

    Poller *_poller;
    const VTW_SocketHandle _handle;
    int _events;

    EventCallback _readCallback;
    EventCallback _writeCallback;
    EventCallback _closeCallback;
    EventCallback _errorCallback;
};

} // namespace net
} // namespace vtw
