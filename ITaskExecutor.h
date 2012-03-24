#ifndef __GS_DVRLIB_TOOLKIT_ITASKEXECUTOR_H__
#define __GS_DVRLIB_TOOLKIT_ITASKEXECUTOR_H__


#include <stingray/toolkit/function.h>
#include <stingray/toolkit/shared_ptr.h>
#include <stingray/threads/Thread.h>


namespace stingray
{

	class task_alive_token
	{
		friend struct ITaskExecutor;

	public:
		struct Value { shared_ptr<Mutex> Sync; Value() : Sync(new Mutex) { } };
		TOOLKIT_DECLARE_PTR(Value);

		task_alive_token() : _value(new Value) { }
		~task_alive_token() { release(); }

		void release()
		{
			if (_value)
			{
				shared_ptr<Mutex> m = _value->Sync;
				MutexLock l(*m);
				_value.reset();
			}
		}

	private:
		ValuePtr	_value;
	};


	struct ITaskExecutor
	{
		virtual void AddTask(const function<void()>& task) = 0;
		virtual void AddTask(const function<void()>& task, const task_alive_token& token) = 0;
		virtual ~ITaskExecutor() {}

		static shared_ptr<ITaskExecutor> Create(const std::string& name, const function<void(const std::exception&)>& exceptionHandler = &ITaskExecutor::DefaultExceptionHandler);

	private:
		static void DefaultExceptionHandler(const std::exception& ex);

	protected:
		static task_alive_token::ValueWeakPtr GetAliveTokenValue(const task_alive_token& t) { return t._value; }

		static void InvokeTask(const function<void()>& task, const task_alive_token::ValueWeakPtr& tokenVal)
		{
			shared_ptr<Mutex> m;
			{
				task_alive_token::ValuePtr v = tokenVal.lock();
				if (!v)
					return;
				m = v->Sync;
			}

			MutexLock tl(*m);
			if (tokenVal.lock())
				task();
		}
	};
	TOOLKIT_DECLARE_PTR(ITaskExecutor);


}


#endif
