#ifndef STINGRAYKIT_THREAD_THREADNAMEAPPENDER_H
#define STINGRAYKIT_THREAD_THREADNAMEAPPENDER_H

#include <stingraykit/log/Logger.h>

namespace stingray
{

	class ThreadNameAppender
	{
		STINGRAYKIT_NONCOPYABLE(ThreadNameAppender);

	private:
		std::string			_threadName;

	public:
		explicit ThreadNameAppender(const std::string& suffix)
			:	_threadName(Thread::GetCurrentThreadName())
		{ Thread::SetCurrentThreadName(StringBuilder() % _threadName % ":" % suffix); }

		~ThreadNameAppender()
		{ STINGRAYKIT_TRY_NO_MESSAGE(Thread::SetCurrentThreadName(_threadName)); }
	};

}

#endif
