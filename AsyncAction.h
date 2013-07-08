#ifndef STINGRAY_TOOLKIT_ASYNCACTION_H
#define STINGRAY_TOOLKIT_ASYNCACTION_H


#include <stingray/threads/Thread.h>
#include <stingray/timer/Timer.h>
#include <stingray/toolkit/ITaskExecutor.h>
#include <stingray/toolkit/shared_ptr.h>
#include <stingray/toolkit/signals.h>
#include <stingray/toolkit/future.h>


namespace stingray
{


	template < typename StateType, typename ResultType >
	class AsyncAction
	{
		typedef	function<ResultType(StateType)>		ReachStateFunc;

	public:
		typedef promise<ResultType> Promise;
		typedef future<ResultType>	Future;
		TOOLKIT_DECLARE_PTR(Promise);

	private:
		struct PromiseRef { PromisePtr Ptr; PromiseRef(): Ptr(new Promise) {} };
		TOOLKIT_DECLARE_PTR(PromiseRef);

		ITaskExecutorPtr				_worker;
		ReachStateFunc					_reachStateFunc;
		ExecutionDeferrerWithTimerPtr	_deferrer;
		PromiseRefPtr					_lastPromiseRef;
		Mutex		  			 	 	_mutex;
		TaskLifeToken 			  		_token;

	public:
		AsyncAction(const ITaskExecutorPtr& worker, const ReachStateFunc& reachStateFunc)
			: _worker(worker), _reachStateFunc(reachStateFunc)
		{ }

		AsyncAction(const ITaskExecutorPtr& worker, const ReachStateFunc& reachStateFunc, size_t deferTimeoutMs)
			: _worker(worker), _reachStateFunc(reachStateFunc), _deferrer(new ExecutionDeferrerWithTimer("asyncActionDeferrer", deferTimeoutMs))
		{ }

		~AsyncAction()
		{ _token.Release(); }

		Future SetState(const StateType& state)
		{
			MutexLock l(_mutex);

			_lastPromiseRef.reset(new PromiseRef);
			PromisePtr promise(_lastPromiseRef->Ptr);

			if (!_deferrer)
				ScheduleReachState(state, _lastPromiseRef.weak());
			else
				_deferrer->Defer(bind(&AsyncAction::ScheduleReachState, this, state, _lastPromiseRef.weak()));

			return promise->get_future();
		}

	private:
		void ScheduleReachState(const StateType& state, const PromiseRefWeakPtr& listenerRefWeak)
		{ _worker->AddTask(bind(&AsyncAction::DoReachState, this, state, listenerRefWeak), _token.GetExecutionTester()); }

		void DoReachState(const StateType& state, const PromiseRefWeakPtr& listenerRefWeak)
		{
			PromiseRefPtr listener_ref = listenerRefWeak.lock();
			if (!listener_ref)
				return;
			try
			{
				ResultType res = _reachStateFunc(state);
				listener_ref->Ptr->set_value(res);
				{
					MutexLock l(_mutex);
					listener_ref->Ptr.reset();
				}
			}
			catch(const std::exception &ex)
			{
				Logger::Error() << "Exception in AsyncAction: " << ex;
				listener_ref->Ptr->set_exception(make_exception_ptr(ex));
			}
		}
	};

	template < typename StateType >
	class AsyncAction<StateType, void>
	{
		typedef	function<void(StateType)>		ReachStateFunc;

	public:
		typedef promise<void>	Promise;
		typedef future<void>	Future;
		TOOLKIT_DECLARE_PTR(Promise);

	private:
		struct PromiseRef { PromisePtr Ptr; PromiseRef(): Ptr(new Promise) {} };
		TOOLKIT_DECLARE_PTR(PromiseRef);

		ITaskExecutorPtr				_worker;
		ReachStateFunc					_reachStateFunc;
		ExecutionDeferrerWithTimerPtr	_deferrer;
		PromiseRefPtr					_lastPromiseRef;
		Mutex							_mutex;
		TaskLifeToken					_token;

	public:
		AsyncAction(const ITaskExecutorPtr& worker, const ReachStateFunc& reachStateFunc)
			: _worker(worker), _reachStateFunc(reachStateFunc)
		{ }

		AsyncAction(const ITaskExecutorPtr& worker, const ReachStateFunc& reachStateFunc, size_t deferTimeoutMs)
			: _worker(worker), _reachStateFunc(reachStateFunc), _deferrer(new ExecutionDeferrerWithTimer("asyncActionDeferrer", deferTimeoutMs))
		{ }

		~AsyncAction()
		{ _token.Release(); }

		Future SetState(const StateType& state)
		{
			MutexLock l(_mutex);
			_lastPromiseRef.reset(new PromiseRef);
			PromisePtr promise(_lastPromiseRef->Ptr);

			if (!_deferrer)
				ScheduleReachState(state, _lastPromiseRef.weak());
			else
				_deferrer->Defer(bind(&AsyncAction::ScheduleReachState, this, state, _lastPromiseRef.weak()));

			return promise->get_future();
		}

	private:
		void ScheduleReachState(const StateType& state, const PromiseRefWeakPtr& listenerRefWeak)
		{ _worker->AddTask(bind(&AsyncAction::DoReachState, this, state, listenerRefWeak), _token.GetExecutionTester()); }

		void DoReachState(const StateType& state, const PromiseRefWeakPtr& listenerRefWeak)
		{
			PromiseRefPtr listener_ref = listenerRefWeak.lock();
			if (!listener_ref)
				return;
			try
			{
				_reachStateFunc(state);
				listener_ref->Ptr->set_value();
				{
					MutexLock l(_mutex);
					listener_ref->Ptr.reset();
				}
			}
			catch(const std::exception &ex)
			{
				Logger::Error() << "Exception in AsyncAction: " << ex;
				listener_ref->Ptr->set_exception(make_exception_ptr(ex));
			}
		}
	};

	template <>
	class AsyncAction<void, void>
	{
		typedef	function<void()>		ReachStateFunc;

	public:
		typedef promise<void>	Promise;
		typedef future<void>	Future;
		TOOLKIT_DECLARE_PTR(Promise);

	private:
		struct PromiseRef { PromisePtr Ptr; PromiseRef(): Ptr(new Promise) {} };
		TOOLKIT_DECLARE_PTR(PromiseRef);

		ITaskExecutorPtr				_worker;
		ReachStateFunc					_reachStateFunc;
		ExecutionDeferrerWithTimerPtr	_deferrer;
		PromiseRefPtr					_lastPromiseRef;
		Mutex							_mutex;
		TaskLifeToken					_token;

	public:
		AsyncAction(const ITaskExecutorPtr& worker, const ReachStateFunc& reachStateFunc)
			: _worker(worker), _reachStateFunc(reachStateFunc)
		{ }

		AsyncAction(const ITaskExecutorPtr& worker, const ReachStateFunc& reachStateFunc, size_t deferTimeoutMs)
			: _worker(worker), _reachStateFunc(reachStateFunc), _deferrer(new ExecutionDeferrerWithTimer("asyncActionDeferrer", deferTimeoutMs))
		{ }

		~AsyncAction()
		{ _token.Release(); }

		Future SetState()
		{
			MutexLock l(_mutex);
			_lastPromiseRef.reset(new PromiseRef);
			PromisePtr promise(_lastPromiseRef->Ptr);

			if (!_deferrer)
				ScheduleReachState(_lastPromiseRef.weak());
			else
				_deferrer->Defer(bind(&AsyncAction::ScheduleReachState, this, _lastPromiseRef.weak()));

			return promise->get_future();
		}

	private:
		void ScheduleReachState(const PromiseRefWeakPtr& listenerRefWeak)
		{ _worker->AddTask(bind(&AsyncAction::DoReachState, this, listenerRefWeak), _token.GetExecutionTester()); }

		void DoReachState(const PromiseRefWeakPtr& listenerRefWeak)
		{
			PromiseRefPtr listener_ref = listenerRefWeak.lock();
			if (!listener_ref)
				return;

			try
			{
				_reachStateFunc();
				listener_ref->Ptr->set_value();
				{
					MutexLock l(_mutex);
					listener_ref->Ptr.reset();
				}
			}
			catch(const std::exception &ex)
			{
				Logger::Error() << "Exception in AsyncAction: " << ex;
				listener_ref->Ptr->set_exception(make_exception_ptr(ex));
			}
		}
	};


}


#endif
