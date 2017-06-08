#ifndef STINGRAYKIT_THREAD_ASYNCACTION_H
#define STINGRAYKIT_THREAD_ASYNCACTION_H

// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <stingraykit/thread/Thread.h>
#include <stingraykit/timer/Timer.h>
#include <stingraykit/thread/ITaskExecutor.h>
#include <stingraykit/shared_ptr.h>
#include <stingraykit/signal/signals.h>
#include <stingraykit/future.h>


namespace stingray
{

	/**
	 * @addtogroup toolkit_threads
	 * @{
	 */

	template < typename StateType, typename ResultType >
	class AsyncAction
	{
		typedef	function<ResultType(StateType)>		ReachStateFunc;

	public:
		typedef promise<ResultType> Promise;
		typedef future<ResultType>	Future;
		STINGRAYKIT_DECLARE_PTR(Promise);

	private:
		struct PromiseRef { PromisePtr Ptr; PromiseRef(): Ptr(new Promise) {} };
		STINGRAYKIT_DECLARE_PTR(PromiseRef);

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

		AsyncAction(const ITaskExecutorPtr& worker, const ReachStateFunc& reachStateFunc, TimeDuration deferTimeout)
			: _worker(worker), _reachStateFunc(reachStateFunc), _deferrer(new ExecutionDeferrerWithTimer("asyncActionDeferrer", deferTimeout))
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
		STINGRAYKIT_DECLARE_PTR(Promise);

	private:
		struct PromiseRef { PromisePtr Ptr; PromiseRef(): Ptr(new Promise) {} };
		STINGRAYKIT_DECLARE_PTR(PromiseRef);

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

		AsyncAction(const ITaskExecutorPtr& worker, const ReachStateFunc& reachStateFunc, TimeDuration deferTimeout)
			: _worker(worker), _reachStateFunc(reachStateFunc), _deferrer(new ExecutionDeferrerWithTimer("asyncActionDeferrer", deferTimeout))
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
		STINGRAYKIT_DECLARE_PTR(Promise);

	private:
		struct PromiseRef { PromisePtr Ptr; PromiseRef(): Ptr(new Promise) {} };
		STINGRAYKIT_DECLARE_PTR(PromiseRef);

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

		AsyncAction(const ITaskExecutorPtr& worker, const ReachStateFunc& reachStateFunc, TimeDuration deferTimeout)
			: _worker(worker), _reachStateFunc(reachStateFunc), _deferrer(new ExecutionDeferrerWithTimer("asyncActionDeferrer", deferTimeout))
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

	/** @} */

}


#endif
