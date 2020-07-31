// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/thread/ThreadPool.h>

#include <stingraykit/diagnostics/ExecutorsProfiler.h>
#include <stingraykit/function/bind.h>

namespace stingray
{

	const TimeDuration ThreadPool::DefaultProfileTimeout = TimeDuration::FromSeconds(10);


	class ThreadPool::WorkerWrapper
	{
	private:
		std::string				_name;
		optional<TimeDuration>	_profileTimeout;
		ExceptionHandler		_exceptionHandler;

		Mutex					_mutex;
		optional<Task>			_task;
		ConditionVariable		_cond;

		Thread					_worker;

	public:
		WorkerWrapper(const std::string& name, const optional<TimeDuration>& profileTimeout, const ExceptionHandler& exceptionHandler, const Task& task)
			:	_name(name),
				_profileTimeout(profileTimeout),
				_exceptionHandler(exceptionHandler),
				_task(task),
				_worker(_name, Bind(&WorkerWrapper::ThreadFunc, this, _1))
		{ }

		bool TryAddTask(const Task& task)
		{
			MutexLock l(_mutex);
			if (_task)
				return false;

			_task = task;
			_cond.Broadcast();
			return true;
		}

	private:
		std::string GetProfilerMessage(const Task& task) const
		{ return StringBuilder() % get_function_name(task) % " in ThreadPool '" % _name % "'"; }

		void ThreadFunc(const ICancellationToken& token)
		{
			MutexLock l(_mutex);
			while (token)
			{
				if (!_task)
				{
					_cond.Wait(_mutex, token);
					continue;
				}

				{
					const Task task = *_task;

					MutexUnlock ul(l);
					ExecuteTask(token, task);
				}

				_task.reset();
			}
		}

		void ExecuteTask(const ICancellationToken& token, const Task& task) const
		{
			try
			{
				if (_profileTimeout)
				{
					AsyncProfiler::Session profilerSession(ExecutorsProfiler::Instance().GetProfiler(), Bind(&WorkerWrapper::GetProfilerMessage, this, wrap_const_ref(task)), *_profileTimeout, AsyncProfiler::NameGetterTag());
					task(token);
				}
				else
					task(token);
			}
			catch (const std::exception& ex)
			{ _exceptionHandler(ex); }
		}
	};


	ThreadPool::ThreadPool(const std::string& name, size_t maxThreads, const optional<TimeDuration>& profileTimeout, const ExceptionHandler& exceptionHandler)
		:	_name(name),
			_maxThreads(maxThreads),
			_profileTimeout(profileTimeout),
			_exceptionHandler(exceptionHandler)
	{ STINGRAYKIT_CHECK(_maxThreads != 0, ArgumentException("maxThreads")); }


	void ThreadPool::Queue(const Task& task)
	{
		MutexLock l(_mutex);
		if (!_task)
		{
			if (!_worker)
				_worker.reset(new Thread(_name + "_0", Bind(&ThreadPool::ThreadFunc, this, _1)));

			_task = task;
			_cond.Broadcast();
			return;
		}

		for (Workers::const_iterator it = _workers.begin(); it != _workers.end(); ++it)
			if ((*it)->TryAddTask(task))
				return;

		STINGRAYKIT_CHECK(_workers.size() + 1 < _maxThreads, "Thread limit exceeded");
		_workers.push_back(make_shared_ptr<WorkerWrapper>(StringBuilder() % _name % "_" % (_workers.size() + 1), _profileTimeout, _exceptionHandler, task));
	}


	void ThreadPool::DefaultExceptionHandler(const std::exception& ex)
	{ Logger::Error() << "Thread pool task exception: " << ex; }


	std::string ThreadPool::GetProfilerMessage(const Task& task) const
	{ return StringBuilder() % get_function_name(task) % " in ThreadPool '" % _name % "'"; }


	void ThreadPool::ThreadFunc(const ICancellationToken& token)
	{
		MutexLock l(_mutex);
		while (token)
		{
			if (!_task)
			{
				_cond.Wait(_mutex, token);
				continue;
			}

			{
				const Task task = *_task;

				MutexUnlock ul(l);
				ExecuteTask(token, task);
			}

			_task.reset();
		}
	}

	void ThreadPool::ExecuteTask(const ICancellationToken& token, const Task& task) const
	{
		try
		{
			if (_profileTimeout)
			{
				AsyncProfiler::Session profilerSession(ExecutorsProfiler::Instance().GetProfiler(), Bind(&ThreadPool::GetProfilerMessage, this, wrap_const_ref(task)), *_profileTimeout, AsyncProfiler::NameGetterTag());
				task(token);
			}
			else
				task(token);
		}
		catch (const std::exception& ex)
		{ _exceptionHandler(ex); }
	}

}
