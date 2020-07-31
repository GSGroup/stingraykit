#ifndef STINGRAYKIT_THREAD_THREADPOOL_H
#define STINGRAYKIT_THREAD_THREADPOOL_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/thread/Thread.h>

namespace stingray
{

	class ThreadPool
	{
		STINGRAYKIT_NONCOPYABLE(ThreadPool);

	public:
		typedef function<void (const ICancellationToken&)>	Task;
		typedef function<void (const std::exception&)>		ExceptionHandler;

	public:
		static const TimeDuration DefaultProfileTimeout;

	private:
		class WorkerWrapper;
		STINGRAYKIT_DECLARE_PTR(WorkerWrapper);

		typedef std::vector<WorkerWrapperPtr>	Workers;

	private:
		std::string				_name;
		size_t					_maxThreads;
		optional<TimeDuration>	_profileTimeout;
		ExceptionHandler		_exceptionHandler;

		Mutex					_mutex;
		Workers					_workers;

	public:
		ThreadPool(const std::string& name, size_t maxThreads, const optional<TimeDuration>& profileTimeout = DefaultProfileTimeout, const ExceptionHandler& exceptionHandler = &DefaultExceptionHandler);

		void Queue(const Task& task);

		static void DefaultExceptionHandler(const std::exception& ex);
	};
	STINGRAYKIT_DECLARE_PTR(ThreadPool);

}

#endif
