//Author: WhiteSheep <260886429@qq.com>
//Created: 2021.8.2
//Description:
#ifndef TWNL_TIMER_H
#define TWNL_TIMER_H

#include <cassert>

#include "Channel.h"
#include "Timestamp.h"

namespace twnl
{

	class Timer : noncopyable
	{
	public:
		typedef std::function<void()> TimerCallback;
		Timer(TimerCallback callback, Timestamp when, Nanosecond interval)
			: callback_(std::move(callback)),
			when_(when),
			interval_(interval),
			repeat_(interval_ > Nanosecond::zero()),
			canceled_(false)
		{
		}

		void run() { if (callback_) callback_(); }
		bool repeat() const { return repeat_; }
		bool expired(Timestamp now) const { return now >= when_; }
		Timestamp when() const { return when_; }
		void restart()
		{
			assert(repeat_);
			when_ += interval_;
		}
		void cancel()
		{
			assert(!canceled_);
			canceled_ = true;
		}
		bool canceled() const { return canceled_; }

	private:
		TimerCallback callback_;
		Timestamp when_;
		const Nanosecond interval_;
		bool repeat_;
		bool canceled_;
	};

}
#endif //TWNL_TIMER_H
