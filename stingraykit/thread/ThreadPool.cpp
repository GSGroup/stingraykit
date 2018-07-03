// Copyright (c) 2011 - 2018, GS Group, https://github.com/GSGroup
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

	class ThreadPool::WorkerWrapper
	{
	private:
		const bool			_profileCalls;

		Mutex				_guard;
		optional<Task>		_task;
		ConditionVariable	_cond;

		const ThreadPtr		_worker;

	public:
		explicit WorkerWrapper(const std::string& name, bool profileCalls = true) :
			_profileCalls(profileCalls), _worker(new Thread(name, bind(&WorkerWrapper::ThreadFunc, this, _1)))
		{ }

		bool TryAddTask(const Task& task)
		{
			MutexLock l(_guard);
			if (_task)
				return false;

			_task = task;
			_cond.Broadcast();
			return true;
		}

	private:
		void ThreadFunc(const ICancellationToken& token)
		{
			MutexLock l(_guard);
			while (token)
			{
				if (!_task)
				{
					_cond.Wait(_guard, token);
					continue;
				}

				STINGRAYKIT_TRY("Task execution failed",
					MutexUnlock ul(l);
					if (_profileCalls)
					{
						AsyncProfiler::Session profilerSession(ExecutorsProfiler::Instance().GetProfiler(), StringBuilder() % get_function_name(*_task) % " in ThreadPool worker", TimeDuration::FromSeconds(10));
						(*_task)(token);
					}
					else
						(*_task)(token);
				);
				_task.reset();
			}
		}
	};


	ThreadPool::ThreadPool(const std::string& name, u32 maxThreads, bool profileCalls) :
		_name(name), _maxThreads(maxThreads), _profileCalls(profileCalls)
	{ }


	void ThreadPool::Queue(const Task& task)
	{
		MutexLock l(_mutex);
		for (Workers::const_iterator it = _workers.begin(); it != _workers.end(); ++it)
			if ((*it)->TryAddTask(task))
				return;

		STINGRAYKIT_CHECK(_workers.size() < _maxThreads, "Thread limit exceeded");
		const WorkerWrapperPtr w(new WorkerWrapper(StringBuilder() % _name % "_" % _workers.size(), _profileCalls));
		_workers.push_back(w);
		STINGRAYKIT_CHECK(w->TryAddTask(task), "Internal ThreadPool error!");
	}

}
