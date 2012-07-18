#ifndef __GS_STINGRAY_TOOLKIT_ASYNCACTION_H__
#define __GS_STINGRAY_TOOLKIT_ASYNCACTION_H__


#include <stingray/threads/Thread.h>
#include <stingray/toolkit/ITaskExecutor.h>
#include <stingray/toolkit/shared_ptr.h>
#include <stingray/toolkit/signals.h>


namespace stingray
{


	template < typename StateType, typename ResultType >
	class AsyncAction
	{
		typedef	function<ResultType(StateType)>		ReachStateFunc;

	public:
		class Listener
		{
			friend class AsyncAction<StateType, ResultType>;

		private:
			shared_ptr<ResultType>		_result;

		public:
			signal<void(const ResultType&)>		Done;

		private:
			Listener()
				: Done(bind(&Listener::DonePopulator, this, _1))
			{ }

			void SetResult(const ResultType& result)
			{
				signal_locker l(Done);
				_result.reset(new ResultType(result));
				Done(*_result);
			}

			void DonePopulator(const function<void(const ResultType&)>& slot)
			{
				if (_result)
					slot(*_result);
			}
		};
		TOOLKIT_DECLARE_PTR(Listener);

	private:
		struct ListenerRef { ListenerPtr Ptr; };
		TOOLKIT_DECLARE_PTR(ListenerRef);

		ITaskExecutorPtr	_worker;
		ReachStateFunc		_reachStateFunc;
		ListenerRefPtr		_lastListenerRef;
		Mutex				_mutex;
		TaskLifeToken		_token;

	public:
		AsyncAction(const ITaskExecutorPtr& worker, const ReachStateFunc& reachStateFunc)
			: _worker(worker), _reachStateFunc(reachStateFunc)
		{ }

		~AsyncAction()
		{ _token.Release(); }

		ListenerPtr SetState(const StateType& state)
		{
			MutexLock l(_mutex);
			_lastListenerRef.reset(new ListenerRef);
			_lastListenerRef->Ptr.reset(new Listener);
			_worker->AddTask(bind(&AsyncAction::DoReachState, this, state, _lastListenerRef.weak()), _token.GetExecutionToken());
			return _lastListenerRef->Ptr;
		}

	private:
		void DoReachState(const StateType& state, const ListenerRefWeakPtr& listenerRefWeak)
		{
			ListenerRefPtr listener_ref = listenerRefWeak.lock();
			if (!listener_ref)
				return;
			ResultType res = _reachStateFunc(state);
			listener_ref->Ptr->SetResult(res);
			listener_ref->Ptr.reset();
		}
	};

	template < typename StateType >
	class AsyncAction<StateType, void>
	{
		typedef	function<void(StateType)>		ReachStateFunc;

	public:
		class Listener
		{
			friend class AsyncAction<StateType, void>;

		private:
			bool		_done;

		public:
			signal<void()>		Done;

		private:
			Listener()
				: _done(false), Done(bind(&Listener::DonePopulator, this, _1))
			{ }

			void SetResult()
			{
				signal_locker l(Done);
				_done = true;
			}

			void DonePopulator(const function<void()>& slot)
			{
				if (_done)
					slot();
			}
		};
		TOOLKIT_DECLARE_PTR(Listener);

	private:
		struct ListenerRef { ListenerPtr Ptr; };
		TOOLKIT_DECLARE_PTR(ListenerRef);

		ITaskExecutorPtr	_worker;
		ReachStateFunc		_reachStateFunc;
		ListenerRefPtr		_lastListenerRef;
		Mutex				_mutex;
		TaskLifeToken		_token;

	public:
		AsyncAction(const ITaskExecutorPtr& worker, const ReachStateFunc& reachStateFunc)
			: _worker(worker), _reachStateFunc(reachStateFunc)
		{ }

		~AsyncAction()
		{ _token.Release(); }

		ListenerPtr SetState(const StateType& state)
		{
			MutexLock l(_mutex);
			_lastListenerRef.reset(new ListenerRef);
			_lastListenerRef->Ptr.reset(new Listener);
			_worker->AddTask(bind(&AsyncAction::DoReachState, this, state, _lastListenerRef.weak()), _token.GetExecutionToken());
			return _lastListenerRef->Ptr;
		}

	private:
		void DoReachState(const StateType& state, const ListenerRefWeakPtr& listenerRefWeak)
		{
			ListenerRefPtr listener_ref = listenerRefWeak.lock();
			if (!listener_ref)
				return;
			_reachStateFunc(state);
			listener_ref->Ptr->SetResult();
			listener_ref->Ptr.reset();
		}
	};

	template <>
	class AsyncAction<void, void>
	{
		typedef	function<void()>		ReachStateFunc;

	public:
		class Listener
		{
			friend class AsyncAction<void, void>;

		private:
			bool		_done;

		public:
			signal<void()>		Done;

		private:
			Listener()
				: _done(false), Done(bind(&Listener::DonePopulator, this, _1))
			{ }

			void SetResult()
			{
				signal_locker l(Done);
				_done = true;
			}

			void DonePopulator(const function<void()>& slot)
			{
				if (_done)
					slot();
			}
		};
		TOOLKIT_DECLARE_PTR(Listener);

	private:
		struct ListenerRef { ListenerPtr Ptr; };
		TOOLKIT_DECLARE_PTR(ListenerRef);

		ITaskExecutorPtr	_worker;
		ReachStateFunc		_reachStateFunc;
		ListenerRefPtr		_lastListenerRef;
		Mutex				_mutex;
		task_alive_token	_token;

	public:
		AsyncAction(const ITaskExecutorPtr& worker, const ReachStateFunc& reachStateFunc)
			: _worker(worker), _reachStateFunc(reachStateFunc)
		{ }

		ListenerPtr SetState()
		{
			MutexLock l(_mutex);
			_lastListenerRef.reset(new ListenerRef);
			_lastListenerRef->Ptr.reset(new Listener);
			_worker->AddTask(bind(&AsyncAction::DoReachState, this, _lastListenerRef.weak()), _token);
			return _lastListenerRef->Ptr;
		}

	private:
		void DoReachState(const ListenerRefWeakPtr& listenerRefWeak)
		{
			ListenerRefPtr listener_ref = listenerRefWeak.lock();
			if (!listener_ref)
				return;
			_reachStateFunc();
			listener_ref->Ptr->SetResult();
			listener_ref->Ptr.reset();
		}
	};


}


#endif
