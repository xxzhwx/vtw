#pragma once
#include "net/poller.h"

#ifdef _MSC_VER

namespace vtw
{
namespace net
{

class Channel;

class SelectPoller : public Poller
{
public:
    SelectPoller();
    virtual ~SelectPoller();

    virtual void Svc();
    virtual void UpdateChannel(Channel* channel);
    virtual void RemoveChannel(Channel* channel);

private:
    VTW_SocketHandle _maxFd;

    fd_set _readSet;
    fd_set _writeSet;
    fd_set _exceptSet;
};

} // namespace net
} // namespace vtw

#endif // _MSC_VER
