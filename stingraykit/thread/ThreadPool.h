#ifndef STINGRAYKIT_THREAD_THREADPOOL_H
#define STINGRAYKIT_THREAD_THREADPOOL_H


#include <vector>

#include <stingraykit/thread/ITaskExecutor.h>


namespace stingray
{

	class ThreadPool
	{
		STINGRAYKIT_NONCOPYABLE(ThreadPool);

		class WorkerWrapper
		{
		private:
			Mutex				_mutex;
			bool				_busy;
			ITaskExecutorPtr	_worker;

		public:
			WorkerWrapper(const std::string& name, bool profileCalls);

			bool TryAddTask(const function<void()>& task);
			bool TryAddTaskWithTester(const function<void()>& task, const FutureExecutionTester& tester);

		private:
			void TaskWrapper(const function<void()>& task);
		};
		STINGRAYKIT_DECLARE_PTR(WorkerWrapper);

		typedef std::vector<WorkerWrapperPtr>	Workers;

	private:
		Mutex			_mutex;
		std::string		_name;
		u32				_maxThreads;
		bool			_profileCalls;
		Workers			_workers;

	public:
		ThreadPool(const std::string& name, u32 maxThreads, bool profileCalls = true);

		void Queue(const function<void()>& task);
		void Queue(const function<void()>& task, const FutureExecutionTester& tester);

	private:
		void DoAddTask(const function<bool(WorkerWrapper*)>& tryAddTaskFunc);
	};
	STINGRAYKIT_DECLARE_PTR(ThreadPool);

}

#endif
