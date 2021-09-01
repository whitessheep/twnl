//Author: WhiteSheep <260886429@qq.com>
//Created: 2021.8.2
//Description:
#ifndef TWNL_TIMERQUEUE_H
#define TWNL_TIMERQUEUE_H

#include <memory>
#include <set>
#include <vector>

#include "Timer.h"
#include "Channel.h"


namespace twnl
{

class TimerQueue: noncopyable
{
public:
	typedef std::function<void()> TimerCallback;
    explicit
    TimerQueue(EventLoop* loop);
    ~TimerQueue();

    Timer* addTimer(TimerCallback cb, Timestamp when, Nanosecond interval);
    void cancelTimer(Timer* timer);

private:
    typedef std::pair<Timestamp, Timer*> Entry;
    typedef std::set<Entry> TimerList;

    void handleRead();
    std::vector<Entry> getExpired(Timestamp now);

private:
    EventLoop* loop_;
    const int timerfd_;
    Channel timerChannel_;
    TimerList timers_;
};

}
#endif //TWNL_TIMERQUEUE_H
