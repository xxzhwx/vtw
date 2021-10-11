#include <cassert>
#include "net/selectpoller.h"
#include "net/channel.h"

#ifdef _MSC_VER

namespace vtw
{
namespace net
{

SelectPoller::SelectPoller()
    :  _maxFd(0)
{
    FD_ZERO(&_readSet);
    FD_ZERO(&_writeSet);
    FD_ZERO(&_exceptSet);
}

SelectPoller::~SelectPoller() = default;

void SelectPoller::Svc()
{
    assert(!_running);

    _running = true;
    _stopped = false;

    static const int interval = 20; // ms
    while (!_stopped)
    {
        HandleTasks();

        // try to poll I/O events
        if (_maxFd == 0)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(interval));
            return;
        }

        // copy sets
        fd_set readSet = _readSet;
        fd_set writeSet = _writeSet;
        fd_set exceptSet = _exceptSet;

        // select
        struct timeval timeout;
        timeout.tv_sec = interval / 1000;
        timeout.tv_usec = (interval % 1000) * 1000;
        int ret = ::select(static_cast<int>(_maxFd + 1), &readSet, &writeSet, &exceptSet, &timeout);
        if (ret == 0) // time out
            continue;

        if (ret == SOCKET_ERROR)
        {
            //TODO call WSAGetLastError to retrieve a specific error code, and log
        }

        // in Win32 platform, the fd_set use {int fd_array[], int count} way to implement.
        for (unsigned int i = 0; i < exceptSet.fd_count; ++i)
        {
            // (FROM MSDN) a socket will be identified in exceptfds if:
            // if processing a connect call (nonblocking), connection attempt failed.
            // OOB data is available for reading (only if SO_OOBINLINE is disabled.

            // Here, we only consider the connecting failed situation.
            Channel *channel = _channels.find(exceptSet.fd_array[i])->second;
            channel->HandleErrorEvent();
        }
        for (unsigned int i = 0; i < readSet.fd_count; ++i)
        {
            Channel *channel = _channels.find(readSet.fd_array[i])->second;
            channel->HandleReadEvent();
        }
        for (unsigned int i = 0; i < writeSet.fd_count; ++i)
        {
            Channel *channel = _channels.find(writeSet.fd_array[i])->second;
            channel->HandleWriteEvent();
        }

        //TODO sleep for (interval - elapsed) ms
    }

    _running = false;
}

void SelectPoller::UpdateChannel(Channel* channel)
{
    Poller::AssertInPollerThread();

    assert(channel->GetPoller() == this); // maybe allowed moving around pollers ?

    const auto handle = channel->Handle();
    bool isNew = false;
    if (_channels.find(handle) == _channels.end())
    {
        _channels[handle] = channel;
        isNew = true;
    }

    if (channel->IsReading())
    {
        FD_SET(handle, &_readSet);
    }
    else
    {
        if (!isNew)
        {
            FD_CLR(handle, &_readSet);
        }
    }

    if (channel->IsWriting())
    {
        FD_SET(handle, &_writeSet);
    }
    else
    {
        if (!isNew)
        {
            FD_CLR(handle, &_writeSet);
        }
    }

    if (!channel->IsNoneEvent())
    {
        FD_SET(handle, &_exceptSet);
    }
    else
    {
        if (!isNew)
        {
            FD_CLR(handle, &_exceptSet);
        }
    }
}

void SelectPoller::RemoveChannel(Channel* channel)
{
    Poller::AssertInPollerThread();

    const auto handle = channel->Handle();
    assert(_channels.find(handle) != _channels.end());
    assert(_channels[handle] == channel);
    assert(channel->IsNoneEvent()); // or disableAll here ?

    size_t n = _channels.erase(handle);
    assert(n == 1);
}

} // namespace net
} // namespace vtw

#endif // _MSC_VER
