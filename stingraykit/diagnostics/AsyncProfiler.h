#ifndef STINGRAYKIT_DIAGNOSTICS_ASYNCPROFILER_H
#define STINGRAYKIT_DIAGNOSTICS_ASYNCPROFILER_H

// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <stingraykit/collection/IntrusiveList.h>
#include <stingraykit/shared_ptr.h>
#include <stingraykit/time/ElapsedTime.h>
#include <stingraykit/timer/Timer.h>

#include <string>

namespace stingray
{

	/**
	 * @addtogroup toolkit_profiling
	 * @{
	 */

	class AsyncProfiler;
	STINGRAYKIT_DECLARE_PTR(AsyncProfiler);

	class AsyncProfiler
	{
	private:
		class MessageHolder
		{
		private:
			std::string _messageHolder;
			const char* _message;

		public:
			MessageHolder(const char* message) : _message(message)
			{ }

			MessageHolder(const std::string& message) : _messageHolder(message)
			{ _message = _messageHolder.c_str(); }

			const char* Get() const
			{ return _message; }
		};

		typedef function<std::string()>	NameGetterFunc;

		class SessionImpl : public IntrusiveListNodeData
		{
			STINGRAYKIT_NONCOPYABLE(SessionImpl);

		private:
			const char*					_name;
			optional<NameGetterFunc>	_nameGetter;
			Backtrace					_backtrace;
			IThreadInfoPtr				_threadInfo;
			u64							_startTime;
			u64							_timeoutTime;

		public:
			SessionImpl(const char* name) :
				_name(name),
				_threadInfo(Thread::GetCurrentThreadInfo()),
				_startTime(TimeEngine::GetMonotonicMicroseconds())
			{ }

			SessionImpl(const optional<NameGetterFunc>& nameGetter) :
				_name(null),
				_nameGetter(nameGetter),
				_threadInfo(Thread::GetCurrentThreadInfo()),
				_startTime(TimeEngine::GetMonotonicMicroseconds())
			{ }

			std::string GetName()
			{
				if (!_name)
					return _nameGetter.get()();
				return _name;
			}

			std::string GetThreadName() const
			{ return _threadInfo->GetName(); }

			std::string GetBacktrace() const
			{ return _backtrace.Get(); }

			u64 GetStartTime() const
			{ return _startTime; }

			u64 GetAbsoluteTimeout() const
			{ return _timeoutTime; }

			void SetAbsoluteTimeout(u64 timeout)
			{ _timeoutTime = timeout; }
		};

	public:
		class Session
		{
		public:
			struct NameGetterTag { };

		private:
			static NamedLogger	s_logger;
			AsyncProfilerPtr	_asyncProfiler;
			MessageHolder		_nameHolder;
			u64					_thresholdMs;
			SessionImpl			_impl;

		public:
			Session(const AsyncProfilerPtr& profiler, const char* name, size_t criticalMs);
			Session(const AsyncProfilerPtr& profiler, const std::string& name, size_t criticalMs);
			Session(const AsyncProfilerPtr& profiler, const NameGetterFunc& nameGetter, size_t criticalMs, const NameGetterTag&);
			~Session();
		};

	private:
		typedef IntrusiveList<SessionImpl> Sessions;

	private:
		static NamedLogger s_logger;
		const u64          _timeoutMicroseconds;
		Mutex              _mutex;
		ConditionVariable  _condition;
		Sessions           _sessions;
		IThreadPtr         _thread;

	public:
		AsyncProfiler(const std::string& threadName);
		~AsyncProfiler();

	private:
		void ThreadFunc(const ICancellationToken& token);
		void AddSession(SessionImpl& session);
		void RemoveSession(SessionImpl& session);
	};


#define STINGRAYKIT_PROFILE_CALL(Profiler_, Milliseconds_, Call_) \
	do { \
		AsyncProfiler::Session detail_session(Profiler_, #Call_, Milliseconds_); \
		Call_; \
	} while (false)

	/** @} */

}


#endif

