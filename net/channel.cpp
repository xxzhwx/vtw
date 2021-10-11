#include <cassert>
#include "net/channel.h"
#include "net/poller.h"

namespace vtw
{
namespace net
{

Channel::Channel(Poller* poller, VTW_SocketHandle handle)
    : _poller(poller)
    , _handle(handle)
    , _events(kPollNone)
    , _readCallback(nullptr)
    , _writeCallback(nullptr)
    , _closeCallback(nullptr)
    , _errorCallback(nullptr)
{

}

Channel::~Channel()
{
    if (_poller->IsInPollerThread())
    {
        assert(!_poller->HasChannel(this));
    }
}

void Channel::Remove()
{
    _poller->RemoveChannel(this);
}

void Channel::update()
{
    _poller->UpdateChannel(this);
}

} // namespace net
} // namespace vtw
