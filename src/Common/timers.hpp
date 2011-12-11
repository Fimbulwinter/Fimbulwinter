#pragma once

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <map>

class TimerManager
{
	struct TimerData
	{
		boost::asio::deadline_timer *timer;
		boost::function<void (int)> callback;

		int timeout;
		bool repeat;
	};

	static std::map<int, struct TimerData> timers_;
	static int timer_nid;
	static boost::asio::io_service *ios; 

public:
	static void Initialize(boost::asio::io_service *i)
	{
		timer_nid = 1;
		ios = i;
	}
	
	static int CreateTimer(int timeout, bool repeat, boost::function<void (int)> callback)
	{
		int id = timer_nid++;

		timers_[id].timeout = timeout;
		timers_[id].repeat = repeat;
		timers_[id].callback = callback;
		timers_[id].timer = new boost::asio::deadline_timer(*ios);

		return id;
	}

	static int CreateStartTimer(int timeout, bool repeat, boost::function<void (int)> callback)
	{
		int id = CreateTimer(timeout, repeat, callback);

		StartTimer(id);

		return id;
	}

	static void StartTimer(int id)
	{
		if (!timers_.count(id))
			return;

		timers_[id].timer->expires_from_now(boost::posix_time::milliseconds(timers_[id].timeout));
		timers_[id].timer->async_wait(boost::bind(&TimerManager::HandleTimer, boost::asio::placeholders::error, id));
	}

	static void HandleTimer(const boost::system::error_code &err, int id)
	{
		if (!timers_.count(id))
			return;
		
		timers_[id].callback(id);

		if (timers_[id].repeat)
			StartTimer(id);
		else
			FreeTimer(id);
	}

	static void FreeTimer(int id)
	{
		if (!timers_.count(id))
			return;

		delete timers_[id].timer;
		timers_.erase(id);
	}

	static void SetTimeout(int id, int timeout)
	{
		if (!timers_.count(id))
			return;

		timers_[id].timeout = timeout;
		timers_[id].timer->expires_from_now(boost::posix_time::milliseconds(timers_[id].timeout));
	}

	static void AddTimeout(int id, int timeout)
	{
		if (!timers_.count(id))
			return;

		timers_[id].timeout += timeout;
		timers_[id].timer->expires_from_now(boost::posix_time::milliseconds(timers_[id].timeout));
	}
};

