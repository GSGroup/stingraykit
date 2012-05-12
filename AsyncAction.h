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
		ITaskExecutorPtr	_worker;
		ReachStateFunc		_reachStateFunc;
		ListenerPtr			_lastListener;
		Mutex				_mutex;
		task_alive_token	_token;

	public:
		AsyncAction(const ITaskExecutorPtr& worker, const ReachStateFunc& reachStateFunc)
			: _worker(worker), _reachStateFunc(reachStateFunc)
		{ }

		ListenerPtr SetState(const StateType& state)
		{
			MutexLock l(_mutex);
			_lastListener.reset(new Listener);
			_worker->AddTask(bind(&AsyncAction::DoReachState, this, state, _lastListener.weak()), _token);
			return _lastListener;
		}

		void ResetState()
		{
			MutexLock l(_mutex);
			_lastListener.reset();
		}

	private:
		void DoReachState(const StateType& state, const ListenerWeakPtr& listenerWeak)
		{
			ListenerPtr listener = listenerWeak.lock();
			if (!listener)
				return;
			ResultType res = _reachStateFunc(state);
			listener->SetResult(res);
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
		ITaskExecutorPtr	_worker;
		ReachStateFunc		_reachStateFunc;
		ListenerPtr			_lastListener;
		Mutex				_mutex;

	public:
		AsyncAction(const ITaskExecutorPtr& worker, const ReachStateFunc& reachStateFunc)
			: _worker(worker), _reachStateFunc(reachStateFunc)
		{ }

		ListenerPtr SetState(const StateType& state)
		{
			MutexLock l(_mutex);
			_lastListener.reset(new Listener);
			_worker->AddTask(bind(&AsyncAction::DoReachState, this, state, _lastListener.weak()));
			return _lastListener;
		}

	private:
		void DoReachState(const StateType& state, const ListenerWeakPtr& listenerWeak)
		{
			ListenerPtr listener = listenerWeak.lock();
			if (!listener)
				return;
			_reachStateFunc(state);
			listener->SetResult();
		}
	};


}


#endif
