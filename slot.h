#ifndef __GS_DVRLIB_TOOLKIT_SLOT_H__
#define __GS_DVRLIB_TOOLKIT_SLOT_H__


#include <stingray/toolkit/Macro.h>
#include <stingray/toolkit/function.h>
#include <stingray/toolkit/ITaskExecutor.h>
#include <stingray/toolkit/bind.h>


/*! \cond GS_INTERNAL */

namespace stingray
{


	namespace Detail
	{
		template < typename Signature >
		class slot_base
		{
		public:
			typedef typename function_info<Signature>::RetType		RetType;
			typedef typename function_info<Signature>::ParamTypes	ParamTypes;
			typedef function<Signature>								FunctionType;

		protected:

		protected:
			ITaskExecutorWeakPtr	_executor;
			FunctionType			_func;

		protected:
			FORCE_INLINE slot_base(const ITaskExecutorPtr& executor, const FunctionType& func) 
				: _executor(executor), _func(func)
			{ 
				if (executor == NULL)
					TOOLKIT_THROW(NullPointerException());
			}

			void DoAddTask(const function<void()>& func) const
			{ 
				ITaskExecutorPtr executor_l = this->_executor.lock();
				if (executor_l != NULL)
					executor_l->AddTask(func); 
			}

			FORCE_INLINE ~slot_base() { }
		};
	}

	template < typename Signature >
	class slot;


	template < >
	class slot < void() > : public Detail::slot_base<void()>
	{
		typedef Detail::slot_base<void()>	base;

	public:
		FORCE_INLINE slot(const ITaskExecutorPtr& executor, const function<void()>& func)
			: base(executor, func)
		{ }

		FORCE_INLINE void operator ()() const { base::DoAddTask(_func); }
	};		


#define TY typename
#define P_(N) typename GetConstReferenceType<T##N>::ValueT p##N

#define DETAIL_TOOLKIT_DECLARE_SLOT(Typenames_, Types_, Decl_, Usage_) \
	template < Typenames_ > \
	class slot < void(Types_) > : public Detail::slot_base<void(Types_)> \
	{ \
		typedef Detail::slot_base<void(Types_)>	base; \
		\
	public: \
		slot(const ITaskExecutorPtr& executor, const function<void(Types_)>& func) \
			: base(executor, func) \
		{ } \
		\
		void operator ()(Decl_) const \
		{ \
			base::DoAddTask(bind(this->_func, Usage_)); \
		} \
	}


	DETAIL_TOOLKIT_DECLARE_SLOT(MK_PARAM(TY T1), MK_PARAM(T1), MK_PARAM(P_(1)), MK_PARAM(p1));
	DETAIL_TOOLKIT_DECLARE_SLOT(MK_PARAM(TY T1, TY T2), MK_PARAM(T1, T2), MK_PARAM(P_(1), P_(2)), MK_PARAM(p1, p2));
	DETAIL_TOOLKIT_DECLARE_SLOT(MK_PARAM(TY T1, TY T2, TY T3), MK_PARAM(T1, T2, T3), MK_PARAM(P_(1), P_(2), P_(3)), MK_PARAM(p1, p2, p3));
	DETAIL_TOOLKIT_DECLARE_SLOT(MK_PARAM(TY T1, TY T2, TY T3, TY T4), MK_PARAM(T1, T2, T3, T4), MK_PARAM(P_(1), P_(2), P_(3), P_(4)), MK_PARAM(p1, p2, p3, p4));
	DETAIL_TOOLKIT_DECLARE_SLOT(MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5), MK_PARAM(T1, T2, T3, T4, T5), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5)), MK_PARAM(p1, p2, p3, p4, p5));
	DETAIL_TOOLKIT_DECLARE_SLOT(MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6), MK_PARAM(T1, T2, T3, T4, T5, T6), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5), P_(6)), MK_PARAM(p1, p2, p3, p4, p5, p6));
	DETAIL_TOOLKIT_DECLARE_SLOT(MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7), MK_PARAM(T1, T2, T3, T4, T5, T6, T7), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5), P_(6), P_(7)), MK_PARAM(p1, p2, p3, p4, p5, p6, p7));
	DETAIL_TOOLKIT_DECLARE_SLOT(MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8), MK_PARAM(T1, T2, T3, T4, T5, T6, T7, T8), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5), P_(6), P_(7), P_(8)), MK_PARAM(p1, p2, p3, p4, p5, p6, p7, p8));
	DETAIL_TOOLKIT_DECLARE_SLOT(MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9), MK_PARAM(T1, T2, T3, T4, T5, T6, T7, T8, T9), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5), P_(6), P_(7), P_(8), P_(9)), MK_PARAM(p1, p2, p3, p4, p5, p6, p7, p8, p9));
	DETAIL_TOOLKIT_DECLARE_SLOT(MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9, TY T10), MK_PARAM(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10), MK_PARAM(P_(1), P_(2), P_(3), P_(4), P_(5), P_(6), P_(7), P_(8), P_(9), P_(10)), MK_PARAM(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10));

#undef P_
#undef TY


}

/*! \endcond */


#endif
