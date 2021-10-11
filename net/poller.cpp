#include <cassert>
#include "net/poller.h"
#include "net/channel.h"
#include "net/selectpoller.h"

namespace vtw
{
namespace net
{

Poller::Poller()
    : _running(false)
    , _stopped(true)
{

}

Poller::~Poller() = default;

void Poller::Start()
{
    assert(!_running);

    std::thread thead(std::bind(&Poller::Svc, this));
    _thread.swap(thead);
}

void Poller::Stop()
{
    assert(!_stopped);
    _stopped = true;
}

bool Poller::HasChannel(Channel* channel) const
{
    AssertInPollerThread();

    ChannelMap::const_iterator it = _channels.find(channel->Handle());
    return it != _channels.end() && it->second == channel;
}

Poller* Poller::NewDefaultPoller()
{
#ifdef _MSC_VER
    return new SelectPoller();
#else
    return nullptr; //TODO implement EpollPoller
#endif
}

void Poller::RunInPoller(const Task& cb)
{
    if (IsInPollerThread())
    {
        cb();
    }
    else
    {
        QueueInPoller(cb);
    }
}

void Poller::QueueInPoller(const Task& cb)
{
    _taskQueue.Put(std::move(cb));
}

bool Poller::IsInPollerThread() const
{
    return _thread.get_id() == std::this_thread::get_id();
}

void Poller::AssertInPollerThread() const
{
    if (!IsInPollerThread())
    {
        assert(false && "Should be in poller thread!!!");
    }
}

void Poller::HandleTasks()
{
    AssertInPollerThread();

    std::vector<Task> tasks;
    if (_taskQueue.TryTakeAll(tasks))
    {
        std::vector<Task>::const_iterator it;
        for (it = tasks.begin(); it != tasks.end(); ++it)
        {
            const Task& cb = (*it);
            cb();
        }
    }
}

} // namespace net
} // namespace vtw
