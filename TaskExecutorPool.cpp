#include <stingray/toolkit/TaskExecutorPool.h>

#include <stingray/toolkit/bind.h>


namespace stingray
{

	TaskExecutorPool::WorkerWrapper::WorkerWrapper(const std::string& name)
		: _busy(false), _worker(ITaskExecutor::Create(name))
	{ }


	bool TaskExecutorPool::WorkerWrapper::TryAddTask(const function<void()>& task)
	{
		MutexLock l(_mutex);
		if (_busy)
			return false;
		_busy = true;
		_worker->AddTask(bind(&TaskExecutorPool::WorkerWrapper::TaskWrapper, this, task));
		return true;
	}


	bool TaskExecutorPool::WorkerWrapper::TryAddTaskWithTester(const function<void()>& task, const FutureExecutionTester& tester)
	{
		MutexLock l(_mutex);
		if (_busy)
			return false;
		_busy = true;
		_worker->AddTask(bind(&TaskExecutorPool::WorkerWrapper::TaskWrapper, this, task), tester);
		return true;
	}


	void TaskExecutorPool::WorkerWrapper::TaskWrapper(const function<void()>& task)
	{
		task();
		{
			MutexLock l(_mutex);
			TOOLKIT_CHECK(_busy, "Internal TaskExecutorPool error!");
			_busy = false;
		}
	}


	///////////////////////////////////////////////////////////////


	TaskExecutorPool::TaskExecutorPool(const std::string& name, size_t maxThreads)
		: _name(name), _maxThreads(maxThreads)
	{ }


	void TaskExecutorPool::AddTask(const function<void()>& task)
	{ DoAddTask(bind(&WorkerWrapper::TryAddTask, _1, task)); }


	void TaskExecutorPool::AddTask(const function<void()>& task, const FutureExecutionTester& tester)
	{ DoAddTask(bind(&WorkerWrapper::TryAddTaskWithTester, _1, task, tester)); }


	void TaskExecutorPool::DoAddTask(const function<bool(WorkerWrapper*)>& tryAddTaskFunc)
	{
		MutexLock l(_mutex);
		for (Workers::const_iterator it = _workers.begin(); it != _workers.end(); ++it)
		{
			if (tryAddTaskFunc(it->get()))
				return;
		}
		if (_workers.size() < _maxThreads)
		{
			WorkerWrapperPtr w(new WorkerWrapper(StringBuilder() % _name % "" % _workers.size()));
			_workers.push_back(w);
			TOOLKIT_CHECK(tryAddTaskFunc(w.get()), "Internal TaskExecutorPool error!");
		}
		else
		{
			TOOLKIT_THROW(NotImplementedException());
		}
	}

}
