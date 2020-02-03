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


		template < typename R, typename... Ts >
		struct IInvokable : public IInvokableBase
		{
			typedef IInvokableBase									BaseType;
			typedef IInvokable<R, Ts...>							MyType;

			typedef R InvokeFunc(MyType*, Ts...);

			IInvokable(GetVTableFunc* func) : BaseType(func) { }
		};


		template < typename FunctorType, typename R, typename... Ts >
		class Invokable : public IInvokable<R, Ts...>
		{
			STINGRAYKIT_NONCOPYABLE(Invokable);

		private:
			typedef IInvokable<R, Ts...>				BaseType;
			typedef Invokable<FunctorType, R, Ts...>	MyType;

			typedef typename BaseType::VTable			VTable;

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
			RetType_ DoInvoke(typename EnableIf<!IsSame<RetType_, void>::Value, const Dummy&>::ValueT, Ts... args)
			{ return FunctorInvoker::InvokeArgs(_func, std::forward<Ts>(args)...); }

			template < typename RetType_ >
			RetType_ DoInvoke(typename EnableIf<IsSame<RetType_, void>::Value, const Dummy&>::ValueT, Ts... args)
			{ FunctorInvoker::InvokeArgs(_func, std::forward<Ts>(args)...); }

			static R Invoke(BaseType* self, Ts... args)
			{ return static_cast<MyType*>(self)->template DoInvoke<R>(Dummy(), std::forward<Ts>(args)...); }

			static void Dtor(IInvokableBase* self)
			{ static_cast<MyType*>(self)->_func.~FunctorType(); }

			static std::string GetName(const IInvokableBase* self)
			{ return get_function_name(static_cast<const MyType*>(self)->_func); }
		};

	}


	template < typename R, typename... Ts >
	class function<R (Ts...)> : public function_info<R (Ts...)>
	{
	public:
		typedef function_info<R (Ts...)>		BaseType;

		typedef typename BaseType::RetType		RetType;
		typedef typename BaseType::ParamTypes	ParamTypes;
		typedef typename BaseType::Signature	Signature;

	private:
		typedef Detail::IInvokable<R, Ts...>		InvokableType;
		typedef self_count_ptr<InvokableType>		InvokableTypePtr;
		typedef typename InvokableType::InvokeFunc	InvokeFunc;

		friend class function_storage;

	private:
		InvokableTypePtr	_invokable;

		function(const InvokableTypePtr& invokable, const Dummy&) : _invokable(invokable)
		{ }

	public:
		template < typename FunctorType >
		function(const FunctorType& func)
			: _invokable(make_self_count_ptr<Detail::Invokable<FunctorType, R, Ts...>>(func))
		{ }

		RetType operator () (Ts... args) const
		{
			InvokeFunc* func = reinterpret_cast<InvokeFunc*>(_invokable->_getVTable().Invoke);
			return func(_invokable.get(), std::forward<Ts>(args)...);
		}

		std::string get_name() const
		{ return "{ function: " + _invokable->_getVTable().GetName(_invokable.get()) + " }"; }
	};


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
