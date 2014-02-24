#ifndef STINGRAY_TOOLKIT_TASKEXECUTORPOOL_H
#define STINGRAY_TOOLKIT_TASKEXECUTORPOOL_H


#include <vector>

#include <stingray/toolkit/ITaskExecutor.h>


namespace stingray
{

	class TaskExecutorPool // Should not inherit the ITaskExecutor!
	{
		TOOLKIT_NONCOPYABLE(TaskExecutorPool);

		class WorkerWrapper
		{
		private:
			Mutex				_mutex;
			bool				_busy;
			ITaskExecutorPtr	_worker;

		public:
			WorkerWrapper(const std::string& name);

			bool TryAddTask(const function<void()>& task);
			bool TryAddTaskWithTester(const function<void()>& task, const FutureExecutionTester& tester);

		private:
			void TaskWrapper(const function<void()>& task);
		};
		TOOLKIT_DECLARE_PTR(WorkerWrapper);

		typedef std::vector<WorkerWrapperPtr>	Workers;

	private:
		Mutex			_mutex;
		std::string		_name;
		u32				_maxThreads;
		Workers			_workers;

	public:
		TaskExecutorPool(const std::string& name, u32 maxThreads);

		void AddTask(const function<void()>& task);
		void AddTask(const function<void()>& task, const FutureExecutionTester& tester);

	private:
		void DoAddTask(const function<bool(WorkerWrapper*)>& tryAddTaskFunc);
	};
	TOOLKIT_DECLARE_PTR(TaskExecutorPool);

}

#endif
