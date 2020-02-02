#ifndef STINGRAYKIT_FUNCTION_FUNCTION_H
#define STINGRAYKIT_FUNCTION_FUNCTION_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/function/FunctorInvoker.h>
#include <stingraykit/function/function_name_getter.h>
#include <stingraykit/self_counter.h>

namespace stingray
{

	/**
	 * @addtogroup toolkit_functions
	 * @{
	 */

#ifndef DOXYGEN_PREPROCESSOR

	template < typename Signature >
	class function;


	namespace Detail
	{

		struct IInvokableBase : public self_counter<IInvokableBase>
		{
			struct VTable
			{
				typedef void DtorFunc(IInvokableBase* self);
				typedef void InvokeFunc(void);
				typedef std::string GetNameFunc(const IInvokableBase* self);

				typedef InvokeFunc*		InvokePtr;
				typedef DtorFunc*		DtorFuncPtr;
				typedef GetNameFunc*	GetNameFuncPtr;

				DtorFuncPtr		Dtor;
				InvokePtr		Invoke;
				GetNameFuncPtr	GetName;

				VTable(DtorFuncPtr dtor, InvokePtr invoke, GetNameFuncPtr getName) : Dtor(dtor), Invoke(invoke), GetName(getName) { }
			};
			typedef VTable GetVTableFunc();
			GetVTableFunc*	_getVTable;

			IInvokableBase(GetVTableFunc* func) : _getVTable(func) { }
			~IInvokableBase() { _getVTable().Dtor(this); }
		};


		template < typename Signature_ >
		struct IInvokable : public IInvokableBase
		{
			typedef IInvokableBase									BaseType;
			typedef Signature_										Signature;
			typedef typename function_info<Signature>::RetType		RetType;
			typedef typename function_info<Signature>::ParamTypes	ParamTypes;

			typedef RetType InvokeFunc(IInvokable* self, const Tuple<ParamTypes>& p);

			IInvokable(GetVTableFunc* func) : BaseType(func) { }
		};


		template < typename Signature, typename FunctorType >
		class Invokable : public IInvokable<Signature>
		{
			STINGRAYKIT_NONCOPYABLE(Invokable);

		private:
			typedef IInvokable<Signature>				BaseType;
			typedef typename BaseType::RetType			RetType;
			typedef typename BaseType::ParamTypes		ParamTypes;
			typedef typename BaseType::VTable			VTable;
			typedef Invokable<Signature, FunctorType>	MyType;

		public:
			typedef typename BaseType::InvokeFunc InvokeFunc;

		private:
			FunctorType	_func;

		public:
			Invokable(const FunctorType& func)
				: BaseType(&MyType::GetVTable), _func(func)
			{ }

			static VTable GetVTable()
			{ return VTable(&MyType::Dtor, reinterpret_cast<typename VTable::InvokePtr>(&MyType::Invoke), &MyType::GetName); }

		protected:
			template < typename RetType_ >
			RetType_ DoInvoke(const Tuple<ParamTypes>& p, typename EnableIf<!IsSame<RetType_, void>::Value, Dummy>::ValueT* = 0)
			{ return FunctorInvoker::Invoke(_func, p); }

			template < typename RetType_ >
			RetType_ DoInvoke(const Tuple<ParamTypes>& p, typename EnableIf<IsSame<RetType_, void>::Value, Dummy>::ValueT* = 0)
			{ FunctorInvoker::Invoke(_func, p); }

			static RetType Invoke(BaseType* self, const Tuple<ParamTypes>& p)
			{ return static_cast<MyType*>(self)->template DoInvoke<RetType>(p); }

			static void Dtor(IInvokableBase* self)
			{ static_cast<MyType*>(self)->_func.~FunctorType(); }

			static std::string GetName(const IInvokableBase* self)
			{ return get_function_name(static_cast<const MyType*>(self)->_func); }
		};

	}


	template < typename Signature >
	class function_base : public function_info<Signature>
	{
	public:
		typedef typename function_info<Signature>::RetType		RetType;
		typedef typename function_info<Signature>::ParamTypes	ParamTypes;

	protected:
		typedef Detail::IInvokable<Signature>					InvokableType;
		typedef self_count_ptr<InvokableType>					InvokableTypePtr;
		typedef typename InvokableType::InvokeFunc				InvokeFunc;

		InvokableTypePtr	_invokable;

	protected:
		~function_base()
		{ }

		template < typename FunctorType >
		function_base(const FunctorType& func)
			: _invokable(new Detail::Invokable<Signature, FunctorType>(func))
		{ }

		RetType Invoke(const Tuple<ParamTypes>& p) const
		{
			InvokeFunc* func = reinterpret_cast<InvokeFunc*>(_invokable->_getVTable().Invoke);
			return func(_invokable.get(), p);
		}

	public:
		std::string get_name() const { return "{ function: " + _invokable->_getVTable().GetName(_invokable.get()) + " }"; }

	protected:
		function_base(const InvokableTypePtr& ptr, Dummy dummy) : _invokable(ptr)
		{ }
	};


	template < typename R >
	class function<R()> : public function_base<R()>
	{
		typedef function_base<R()> BaseType;
		typedef typename BaseType::InvokableTypePtr InvokableTypePtr;

		friend class function_storage;

	private:
		function(const InvokableTypePtr& invokable, Dummy dummy) : BaseType(invokable, dummy)
		{ }

	public:
		template < typename FunctorType >
		function(const FunctorType& func)
			: function_base<R()>(func)
		{ }

		R operator () () const
		{
			Tuple<TypeList<>::type> p;
			return this->Invoke(p);
		}
	};


#define TY typename
#define PT(N_) typename GetTypeListItem<ParamTypes, N_ - 1>::ValueT

#define DETAIL_STINGRAYKIT_DECLARE_FUNCTION(ParamsCount_, ParamTypenames_, ParamTypes_, ParamDecl_, ParamUsage_, ParamsFromTypeList_) \
	template < typename R, ParamTypenames_ > \
	class function<R(ParamTypes_)> : public function_base<R(ParamTypes_)> \
	{ \
		typedef function_base<R(ParamTypes_)> BaseType; \
		typedef typename BaseType::InvokableTypePtr InvokableTypePtr; \
		friend class function_storage; \
	private: \
		function(const InvokableTypePtr& invokable, Dummy dummy) : BaseType(invokable, dummy) \
		{ } \
	public: \
		template < typename FunctorType > \
		function(const FunctorType& func) \
			: function_base<R(ParamTypes_)>(func) \
		{ } \
		\
		R operator () (ParamDecl_) const \
		{  \
			Tuple<typename TypeList<ParamTypes_>::type> p(ParamUsage_); \
			return this->Invoke(p); \
		} \
	}

	DETAIL_STINGRAYKIT_DECLARE_FUNCTION(1, MK_PARAM(TY T1), MK_PARAM(T1), MK_PARAM(T1 p1), MK_PARAM(p1), MK_PARAM(PT(1)));
	DETAIL_STINGRAYKIT_DECLARE_FUNCTION(2, MK_PARAM(TY T1, TY T2), MK_PARAM(T1, T2), MK_PARAM(T1 p1, T2 p2), MK_PARAM(p1, p2), MK_PARAM(PT(1), PT(2)));
	DETAIL_STINGRAYKIT_DECLARE_FUNCTION(3, MK_PARAM(TY T1, TY T2, TY T3), MK_PARAM(T1, T2, T3), MK_PARAM(T1 p1, T2 p2, T3 p3), MK_PARAM(p1, p2, p3), MK_PARAM(PT(1), PT(2), PT(3)));
	DETAIL_STINGRAYKIT_DECLARE_FUNCTION(4, MK_PARAM(TY T1, TY T2, TY T3, TY T4), MK_PARAM(T1, T2, T3, T4), MK_PARAM(T1 p1, T2 p2, T3 p3, T4 p4), MK_PARAM(p1, p2, p3, p4), MK_PARAM(PT(1), PT(2), PT(3), PT(4)));
	DETAIL_STINGRAYKIT_DECLARE_FUNCTION(5, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5), MK_PARAM(T1, T2, T3, T4, T5), MK_PARAM(T1 p1, T2 p2, T3 p3, T4 p4, T5 p5), MK_PARAM(p1, p2, p3, p4, p5), MK_PARAM(PT(1), PT(2), PT(3), PT(4), PT(5)));
	DETAIL_STINGRAYKIT_DECLARE_FUNCTION(6, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6), MK_PARAM(T1, T2, T3, T4, T5, T6), MK_PARAM(T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6), MK_PARAM(p1, p2, p3, p4, p5, p6), MK_PARAM(PT(1), PT(2), PT(3), PT(4), PT(5), PT(6)));
	DETAIL_STINGRAYKIT_DECLARE_FUNCTION(7, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7), MK_PARAM(T1, T2, T3, T4, T5, T6, T7), MK_PARAM(T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6, T7 p7), MK_PARAM(p1, p2, p3, p4, p5, p6, p7), MK_PARAM(PT(1), PT(2), PT(3), PT(4), PT(5), PT(6), PT(7)));
	DETAIL_STINGRAYKIT_DECLARE_FUNCTION(8, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8), MK_PARAM(T1, T2, T3, T4, T5, T6, T7, T8), MK_PARAM(T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6, T7 p7, T8 p8), MK_PARAM(p1, p2, p3, p4, p5, p6, p7, p8), MK_PARAM(PT(1), PT(2), PT(3), PT(4), PT(5), PT(6), PT(7), PT(8)));
	DETAIL_STINGRAYKIT_DECLARE_FUNCTION(9, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9), MK_PARAM(T1, T2, T3, T4, T5, T6, T7, T8, T9), MK_PARAM(T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6, T7 p7, T8 p8, T9 p9), MK_PARAM(p1, p2, p3, p4, p5, p6, p7, p8, p9), MK_PARAM(PT(1), PT(2), PT(3), PT(4), PT(5), PT(6), PT(7), PT(8), PT(9)));
	DETAIL_STINGRAYKIT_DECLARE_FUNCTION(10, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9, TY T10), MK_PARAM(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10), MK_PARAM(T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6, T7 p7, T8 p8, T9 p9, T10 p10), MK_PARAM(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10), MK_PARAM(PT(1), PT(2), PT(3), PT(4), PT(5), PT(6), PT(7), PT(8), PT(9), PT(10)));

#undef PT
#undef TY

	class function_storage
	{
	private:
		self_count_ptr<Detail::IInvokableBase>	_invokable;

	public:
		template < typename Signature >
		explicit function_storage(const function<Signature>& func) : _invokable(func._invokable)
		{ }

		template < typename Signature >
		function<Signature> ToFunction() const
		{
			typedef typename function<Signature>::InvokableTypePtr TargetInvokablePtr;
			return function<Signature>(TargetInvokablePtr(_invokable, static_cast_tag()), Dummy());
		}
	};

#else

	/**
	 * @brief Function object
	 * @par Example:
	 * @code
	 * double f(char c, int i)
	 * {
	 *     return (c - 'a') * i + 0.5;
	 * }
	 *
	 * int main()
	 * {
	 *     function<double(char, int)> func(&f);
	 *     std::cout << func('c', 3) << std::endl; // 9.5
	 *     return 0;
	 * }
	 * @endcode
	 * @tparam Signature The signature of the function object (e.g. 'bool(std::string)')
	 */
	template < typename Signature >
	class function<Signature> : public function_info<Signature>
	{
	public:
		/** @param[in] func Callable object or pointer to function that will be held inside the function object */
		template < typename FunctorType >
		function(const FunctorType& func);

		/** @param[in] parameters The parameters that will be passed to the inner functor */
		RetType operator ()(Parameters... parameters) const;

		/** @return A string representation of the inner functor. May be used for identifying the function object. */
		std::string get_name() const;
	};

#endif

	/** @} */

}

#endif
