#pragma once
#include <vector>
#include <map>
#include <atomic>
#include <thread>
#include "base/noncopyable.h"
#include "base/lockedqueue.h"
#include "net/socketdatatype.h"

namespace vtw
{
namespace net
{

class Channel;

class Poller : noncopyable
{
public:
    using ChannelList = std::vector<Channel*>;
    using Task = std::function<void()>;

    Poller();
    virtual ~Poller();

    void Start();

    void Stop();

    // The poller thread routine, polls I/O events and handle pending tasks.
    virtual void Svc() = 0;

    // Updates the interested I/O events.
    virtual void UpdateChannel(Channel* channel) = 0;

    // Remove the channel.
    virtual void RemoveChannel(Channel* channel) = 0;

    // Query if owned the channel.
    virtual bool HasChannel(Channel* channel) const;

    static Poller* NewDefaultPoller();

    void RunInPoller(const Task& cb);
    void QueueInPoller(const Task& cb);

    bool IsInPollerThread() const;
    void AssertInPollerThread() const;

protected:
    void HandleTasks();

    using ChannelMap = std::map<VTW_SocketHandle, Channel*>;
    ChannelMap _channels;

    LockedQueue<Task> _taskQueue;

    std::atomic_bool _running;
    std::atomic_bool _stopped;
    std::thread _thread;
};

} // namespace net
} // namespace vtw
