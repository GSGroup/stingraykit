// Copyright (c) 2011 - 2015, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/thread/ThreadPool.h>

#include <stingraykit/log/Logger.h>
#include <stingraykit/function/bind.h>


namespace stingray
{

	ThreadPool::WorkerWrapper::WorkerWrapper(const std::string& name, bool profileCalls)
		: _busy(false), _worker(ITaskExecutor::Create(name, &ITaskExecutor::DefaultExceptionHandler, profileCalls))
	{ }


	bool ThreadPool::WorkerWrapper::TryAddTask(const function<void()>& task)
	{
		MutexLock l(_mutex);
		if (_busy)
			return false;
		_busy = true;
		_worker->AddTask(bind(&ThreadPool::WorkerWrapper::TaskWrapper, this, task));
		return true;
	}


	bool ThreadPool::WorkerWrapper::TryAddTaskWithTester(const function<void()>& task, const FutureExecutionTester& tester)
	{
		MutexLock l(_mutex);
		if (_busy)
			return false;
		_busy = true;
		_worker->AddTask(bind(&ThreadPool::WorkerWrapper::TaskWrapper, this, task), tester);
		return true;
	}


	void ThreadPool::WorkerWrapper::TaskWrapper(const function<void()>& task)
	{
		STINGRAYKIT_TRY("Couldn't execute task", task());
		{
			MutexLock l(_mutex);
			STINGRAYKIT_CHECK(_busy, "Internal TaskExecutorPool error!");
			_busy = false;
		}
	}


	///////////////////////////////////////////////////////////////


	ThreadPool::ThreadPool(const std::string& name, u32 maxThreads, bool profileCalls)
		: _name(name), _maxThreads(maxThreads), _profileCalls(profileCalls)
	{ }


	void ThreadPool::Queue(const function<void()>& task)
	{ DoAddTask(bind(&WorkerWrapper::TryAddTask, _1, task)); }


	void ThreadPool::Queue(const function<void()>& task, const FutureExecutionTester& tester)
	{ DoAddTask(bind(&WorkerWrapper::TryAddTaskWithTester, _1, task, tester)); }


	void ThreadPool::DoAddTask(const function<bool(WorkerWrapper*)>& tryAddTaskFunc)
	{
		MutexLock l(_mutex);
		for (Workers::const_iterator it = _workers.begin(); it != _workers.end(); ++it)
		{
			if (tryAddTaskFunc(it->get()))
				return;
		}
		if (_workers.size() < _maxThreads)
		{
			WorkerWrapperPtr w(new WorkerWrapper(StringBuilder() % _name % "_" % _workers.size(), _profileCalls));
			_workers.push_back(w);
			STINGRAYKIT_CHECK(tryAddTaskFunc(w.get()), "Internal TaskExecutorPool error!");
		}
		else
		{
			STINGRAYKIT_THROW(NotImplementedException());
		}
	}

}
