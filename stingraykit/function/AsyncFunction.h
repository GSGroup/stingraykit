#ifndef STINGRAYKIT_FUNCTION_ASYNC_FUNCTION_H
#define STINGRAYKIT_FUNCTION_ASYNC_FUNCTION_H


#include <stingraykit/thread/ITaskExecutor.h>
#include <stingraykit/Macro.h>
#include <stingraykit/function/bind.h>
#include <stingraykit/function/function.h>
#include <stingraykit/future.h>



namespace stingray
{

	/**
	 * @addtogroup toolkit_functions
	 * @{
	 */

	namespace Detail
	{
		template < typename Signature, bool HasReturnType = !SameType<typename function_info<Signature>::RetType, void>::Value >
		class AsyncFunctionBase
		{
		public:
			typedef void											RetType;
			typedef typename function_info<Signature>::ParamTypes	ParamTypes;
			typedef function<Signature>								FunctionType;

		protected:
			ITaskExecutorPtr	_executor;
			FunctionType		_func;
			TaskLifeToken		_token;

		protected:
			inline AsyncFunctionBase(const ITaskExecutorPtr& executor, const FunctionType& func)
				: _executor(STINGRAYKIT_REQUIRE_NOT_NULL(executor)), _func(func)
			{ }

			RetType DoAddTask(const function<void()>& func) const
			{ _executor->AddTask(func, _token.GetExecutionTester()); }

			inline ~AsyncFunctionBase() { }

		public:
			TaskLifeToken GetToken() const { return _token; }
		};


		template < typename Signature >
		class AsyncFunctionBase<Signature, true>
		{
		public:
			typedef typename function_info<Signature>::RetType		AsyncRetType;
			typedef promise<AsyncRetType>							PromiseType;
			STINGRAYKIT_DECLARE_PTR(PromiseType);

			typedef future<AsyncRetType>							RetType;
			typedef typename function_info<Signature>::ParamTypes	ParamTypes;
			typedef function<Signature>								FunctionType;

		protected:
			ITaskExecutorPtr	_executor;
			FunctionType		_func;
			TaskLifeToken		_token;

		protected:
			inline AsyncFunctionBase(const ITaskExecutorPtr& executor, const FunctionType& func)
				: _executor(STINGRAYKIT_REQUIRE_NOT_NULL(executor)), _func(func)
			{ }

			RetType DoAddTask(const function<AsyncRetType()>& func) const
			{
				PromiseTypePtr promise(new PromiseType);
				_executor->AddTask(bind(&AsyncFunctionBase::FuncWrapper, func, promise), _token.GetExecutionTester());
				return promise->get_future();
			}

			inline ~AsyncFunctionBase() { }

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
	class AsyncFunction;


	template < typename R >
	class AsyncFunction < R() > : public Detail::AsyncFunctionBase<R()>
	{
		typedef Detail::AsyncFunctionBase<R()>	base;

	public:
		inline AsyncFunction(const ITaskExecutorPtr& executor, const function<R()>& func) :
			base(executor, func)
		{ }

		inline typename base::RetType  operator ()() const { return base::DoAddTask(base::_func); }
	};


#define TY typename
#define P_(N) typename GetConstReferenceType<T##N>::ValueT p##N

#define DETAIL_STINGRAYKIT_DECLARE_ASYNC_FUNCTION(Typenames_, Types_, Decl_, Usage_) \
	template < typename R, Typenames_ > \
	class AsyncFunction < R(Types_) > : public Detail::AsyncFunctionBase<R(Types_)> \
	{ \
		typedef Detail::AsyncFunctionBase<R(Types_)>	base; \
		\
	public: \
		AsyncFunction(const ITaskExecutorPtr& executor, const function<R(Types_)>& func) \
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
