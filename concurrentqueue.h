#pragma once

#include <deque>
#include <mutex>
#include <condition_variable>
#include "message.h"

template <class T>
class ConcurrentQueue
{
public:
	void Add(const T& value)
	{
		std::lock_guard<std::mutex> lock(m_);

		q_.push_back(value);
		cv_.notify_one();
	}

	bool Take(T& value, int timeout)
	{
		std::unique_lock<std::mutex> lock(m_);

		while (q_.size() == 0)
		{
			if (timeout < 0)
			{
				cv_.wait(lock);
			}
			else
			{
				std::chrono::milliseconds ms(timeout);
				if (std::cv_status::timeout == cv_.wait_for(lock, ms))
					return false;
			}
		}

		value = q_.front();
		q_.pop_front();
		
		return true;
	}

private:
	typedef std::deque<T> Queue;
	Queue q_;
	std::mutex m_;
	std::condition_variable cv_;
};

typedef ConcurrentQueue<Message> MsgQueue;

