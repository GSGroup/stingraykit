#ifndef STINGRAY_TOOLKIT_TASK_ALIVE_TOKEN_H
#define STINGRAY_TOOLKIT_TASK_ALIVE_TOKEN_H


#include <stingray/threads/Thread.h>
#include <stingray/toolkit/Final.h>
#include <stingray/toolkit/self_counter.h>


namespace stingray
{

	namespace Detail
	{
		struct TaskLifeTokenImpl : public self_counter<TaskLifeTokenImpl>
		{
		private:
			Mutex	_sync;
			bool	_alive;

		public:
			TaskLifeTokenImpl() : _alive(true)
			{}
			Mutex& GetMutex() 		{ return _sync; }
			bool IsAlive() const	{ return _alive; }
			void Kill()				{ _alive = false; }
		};
		TOOLKIT_DECLARE_SELF_COUNT_PTR(TaskLifeTokenImpl);
	};


	struct FutureExecutionTester;
	struct LocalExecutionGuard : public safe_bool<LocalExecutionGuard>
	{
		TOOLKIT_NONCOPYABLE(LocalExecutionGuard);

	private:
		bool									_allow;
		Detail::TaskLifeTokenImplSelfCountPtr	_impl;

	public:
		LocalExecutionGuard() : _allow(false)
		{}
		~LocalExecutionGuard()
		{
			if (_impl)
				_impl->GetMutex().Unlock();
		}
		bool boolean_test() const	{ return _allow; }

	private:
		friend struct FutureExecutionTester;
		void SetImpl(const Detail::TaskLifeTokenImplSelfCountPtr& impl)
		{
			if (!impl)
			{
				_allow = true;
				return;
			}
			_impl = impl;
			_impl->GetMutex().Lock();
			_allow = _impl->IsAlive();
			if (!_allow)
			{
				_impl->GetMutex().Unlock();
				_impl.reset();
			}
		}
	};


	struct FutureExecutionTester
	{
	private:
		typedef Detail::TaskLifeTokenImplSelfCountPtr ImplPtr;
		ImplPtr _impl;

	public:
		FutureExecutionTester(const NullPtrType&) // always allows func execution
		{}

		FutureExecutionTester(const Detail::TaskLifeTokenImplSelfCountPtr& impl) : _impl(impl)
		{}

		LocalExecutionGuard& Execute(LocalExecutionGuard& token) const { token.SetImpl(_impl); return token; }
	};


	class TaskLifeToken : TOOLKIT_FINAL(TaskLifeToken)
	{
	private:
		Detail::TaskLifeTokenImplSelfCountPtr _impl;

	public:
		TaskLifeToken() : _impl(new Detail::TaskLifeTokenImpl)
		{}

		~TaskLifeToken()
		{}

		void Release()
		{
			MutexLock l(_impl->GetMutex());
			_impl->Kill();
		}

		TaskLifeToken& Reset()
		{
			Release();
			return (*this = TaskLifeToken());
		}

		FutureExecutionTester GetExecutionTester() const
		{ return FutureExecutionTester(_impl); }
	};
	TOOLKIT_DECLARE_PTR(TaskLifeToken);

	//void InvokeTask(const function<void ()>& task, const TaskLifeToken::ValueWeakPtr& tokenValue);


	template < typename Signature >
	class function_with_token;

#define DETAIL_TOOLKIT_DECLARE_FUNCTION_WITH_TOKEN(ParamTypenames_, ParamTypes_, ParamDecl_, ParamUsage_) \
	template < ParamTypenames_ > \
	class function_with_token<void(ParamTypes_)> : public function_info<void(ParamTypes_)> \
	{ \
		typedef function<void(ParamTypes_)>	FuncType; \
	private: \
		FuncType				_func; \
		FutureExecutionTester	_execTester; \
	public: \
		function_with_token(const FuncType& func, const FutureExecutionTester& execTester) \
			: _func(func), _execTester(execTester) \
		{ } \
		inline void operator ()(ParamDecl_) const \
		{ \
			LocalExecutionGuard guard; \
			if (_execTester.Execute(guard)) \
				_func(ParamUsage_); \
		} \
		std::string get_name() const { return _func.get_name(); } \
	}

#define TY typename
	DETAIL_TOOLKIT_DECLARE_FUNCTION_WITH_TOKEN(/**/, /**/, /**/, /**/);
	DETAIL_TOOLKIT_DECLARE_FUNCTION_WITH_TOKEN(MK_PARAM(TY T1), MK_PARAM(T1), MK_PARAM(T1 p1), MK_PARAM(p1));
	DETAIL_TOOLKIT_DECLARE_FUNCTION_WITH_TOKEN(MK_PARAM(TY T1, TY T2), MK_PARAM(T1, T2), MK_PARAM(T1 p1, T2 p2), MK_PARAM(p1, p2));
	DETAIL_TOOLKIT_DECLARE_FUNCTION_WITH_TOKEN(MK_PARAM(TY T1, TY T2, TY T3), MK_PARAM(T1, T2, T3), MK_PARAM(T1 p1, T2 p2, T3 p3), MK_PARAM(p1, p2, p3));
	DETAIL_TOOLKIT_DECLARE_FUNCTION_WITH_TOKEN(MK_PARAM(TY T1, TY T2, TY T3, TY T4), MK_PARAM(T1, T2, T3, T4), MK_PARAM(T1 p1, T2 p2, T3 p3, T4 p4), MK_PARAM(p1, p2, p3, p4));
	DETAIL_TOOLKIT_DECLARE_FUNCTION_WITH_TOKEN(MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5), MK_PARAM(T1, T2, T3, T4, T5), MK_PARAM(T1 p1, T2 p2, T3 p3, T4 p4, T5 p5), MK_PARAM(p1, p2, p3, p4, p5));
#undef TY

}


#endif
