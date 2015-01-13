#ifndef STINGRAYKIT_FUNCTION_ASYNC_FUNCTION_H
#define STINGRAYKIT_FUNCTION_ASYNC_FUNCTION_H


#include <stingray/toolkit/thread/ITaskExecutor.h>
#include <stingray/toolkit/Macro.h>
#include <stingray/toolkit/function/bind.h>
#include <stingray/toolkit/function/function.h>
#include <stingray/toolkit/future.h>



namespace stingray
{

	/**
	 * @addtogroup toolkit_functions
	 * @{
	 */

	namespace Detail
	{
		template < typename Signature, bool HasReturnType = !SameType<typename function_info<Signature>::RetType, void>::Value >
		class async_function_base
		{
		public:
			typedef void											RetType;
			typedef typename function_info<Signature>::ParamTypes	ParamTypes;
			typedef function<Signature>								FunctionType;

		protected:
			ITaskExecutorWeakPtr	_executor;
			FunctionType			_func;
			TaskLifeToken			_token;

		protected:
			inline async_function_base(const ITaskExecutorPtr& executor, const FunctionType& func)
				: _executor(STINGRAYKIT_REQUIRE_NOT_NULL(executor)), _func(func)
			{ }

			RetType DoAddTask(const function<void()>& func) const
			{
				ITaskExecutorPtr executor_l = this->_executor.lock();
				if (executor_l)
					executor_l->AddTask(func, _token.GetExecutionTester());
			}

			inline ~async_function_base() { }

		public:
			TaskLifeToken GetToken() const { return _token; }
		};


		template < typename Signature >
		class async_function_base<Signature, true>
		{
		public:
			typedef typename function_info<Signature>::RetType		AsyncRetType;
			typedef promise<AsyncRetType>							PromiseType;
			STINGRAYKIT_DECLARE_PTR(PromiseType);

			typedef future<AsyncRetType>							RetType;
			typedef typename function_info<Signature>::ParamTypes	ParamTypes;
			typedef function<Signature>								FunctionType;

		protected:
			ITaskExecutorWeakPtr	_executor;
			FunctionType			_func;
			TaskLifeToken			_token;

		protected:
			inline async_function_base(const ITaskExecutorPtr& executor, const FunctionType& func)
				: _executor(STINGRAYKIT_REQUIRE_NOT_NULL(executor)), _func(func)
			{ }

			RetType DoAddTask(const function<AsyncRetType()>& func) const
			{
				PromiseTypePtr promise(new PromiseType);
				ITaskExecutorPtr executor_l = this->_executor.lock();
				if (executor_l)
					executor_l->AddTask(bind(&async_function_base::FuncWrapper, func, promise), _token.GetExecutionTester());
				return promise->get_future();
			}

			inline ~async_function_base() { }

		private:
			static void FuncWrapper(const function<AsyncRetType()>& func, const PromiseTypePtr& promise)
			{
				STINGRAYKIT_REQUIRE_NOT_NULL(promise);
				try
				{ promise->set_value(func()); }
				catch (const std::exception& ex)
				{ promise->set_exception(make_exception_ptr(ex)); }
			}

		public:
			TaskLifeToken GetToken() const { return _token; }
		};
	}

	template < typename Signature >
	class async_function;


	template < typename R >
	class async_function < R() > : public Detail::async_function_base<R()>
	{
		typedef Detail::async_function_base<R()>	base;

	public:
		inline async_function(const ITaskExecutorPtr& executor, const function<R()>& func)
			: base(executor, func)
		{ }

		inline typename base::RetType  operator ()() const { return base::DoAddTask(base::_func); }
	};


#define TY typename
#define P_(N) typename GetConstReferenceType<T##N>::ValueT p##N

#define DETAIL_STINGRAYKIT_DECLARE_ASYNC_FUNCTION(Typenames_, Types_, Decl_, Usage_) \
	template < typename R, Typenames_ > \
	class async_function < R(Types_) > : public Detail::async_function_base<R(Types_)> \
	{ \
		typedef Detail::async_function_base<R(Types_)>	base; \
		\
	public: \
		async_function(const ITaskExecutorPtr& executor, const function<R(Types_)>& func) \
			: base(executor, func) \
		{ } \
		\
		typename base::RetType operator ()(Decl_) const \
		{ \
			return base::DoAddTask(bind(this->_func, Usage_)); \
		} \
	}


	DETAIL_STINGRAYKIT_DECLARE_ASYNC_FUNCTION(MK_PARAM(TY T1), MK_PARAM(T1), MK_PARAM(P_(1)), MK_PARAM(p1));
	DETAIL_STINGRAYKIT_DECLARE_ASYNC_FUNCTION(MK_PARAM(TY T1, TY T2), MK_PARAM(T1, T2), MK_PARAM(P_(1), P_(2)), MK_PARAM(p1, p2));
	DETAIL_STINGRAYKIT_DECLARE_ASYNC_FUNCTION(MK_PARAM(TY T1, TY T2, TY T3), MK_PARAM(T1, T2, T3), MK_PARAM(P_(1), P_(2), P_(3)), MK_PARAM(p1, p2, p3));
	DETAIL_STINGRAYKIT_DECLARE_ASYNC_FUNCTION(MK_PARAM(TY T1, TY T2, TY T3, TY T4), MK_PARAM(T1, T2, T3, T4), MK_PARAM(P_(1), P_(2), P_(3), P_(4)), MK_PARAM(p1, p2, p3, p4));
	DETAIL_STINGRAYKIT_DECLARE_ASYNC_FUNCTION(MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5), MK_PARAM(T1, T2, T3, T4, T5), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5)), MK_PARAM(p1, p2, p3, p4, p5));
	DETAIL_STINGRAYKIT_DECLARE_ASYNC_FUNCTION(MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6), MK_PARAM(T1, T2, T3, T4, T5, T6), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5), P_(6)), MK_PARAM(p1, p2, p3, p4, p5, p6));
	DETAIL_STINGRAYKIT_DECLARE_ASYNC_FUNCTION(MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7), MK_PARAM(T1, T2, T3, T4, T5, T6, T7), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5), P_(6), P_(7)), MK_PARAM(p1, p2, p3, p4, p5, p6, p7));
	DETAIL_STINGRAYKIT_DECLARE_ASYNC_FUNCTION(MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8), MK_PARAM(T1, T2, T3, T4, T5, T6, T7, T8), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5), P_(6), P_(7), P_(8)), MK_PARAM(p1, p2, p3, p4, p5, p6, p7, p8));
	DETAIL_STINGRAYKIT_DECLARE_ASYNC_FUNCTION(MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9), MK_PARAM(T1, T2, T3, T4, T5, T6, T7, T8, T9), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5), P_(6), P_(7), P_(8), P_(9)), MK_PARAM(p1, p2, p3, p4, p5, p6, p7, p8, p9));
	DETAIL_STINGRAYKIT_DECLARE_ASYNC_FUNCTION(MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9, TY T10), MK_PARAM(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5), P_(6), P_(7), P_(8), P_(9), P_(10)), MK_PARAM(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10));

#undef P_
#undef TY


	/** @} */

}



#endif
