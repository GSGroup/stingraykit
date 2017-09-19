#ifndef STINGRAYKIT_FUNCTION_FUNCTIONAL_H
#define STINGRAYKIT_FUNCTION_FUNCTIONAL_H

// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/function/function.h>
#include <stingraykit/PerfectForwarding.h>

namespace stingray
{

	/**
	 * @addtogroup toolkit_functions
	 * @{
	 */

	namespace Detail
	{
		template < typename F >
		class NotFunc : public function_info<F>
		{
			F	_f;

		public:
			NotFunc(const F& f) : _f(f)
			{ }

			STINGRAYKIT_PERFECT_FORWARDING(bool, operator (), Do)

		private:
			template< typename ParamTypeList >
			bool Do(const Tuple<ParamTypeList>& params) const
			{ return !FunctorInvoker::Invoke(_f, params); }
		};
	}

	template < typename F >
	Detail::NotFunc<F> not_(const F& f) { return Detail::NotFunc<F>(f); }


	namespace Detail
	{
		template < typename F >
		class NegateFunc : public function_info<F>
		{
			typedef typename function_info<F>::RetType		Ret;

			F	_f;

		public:
			NegateFunc(const F& f) : _f(f)
			{ }

			STINGRAYKIT_PERFECT_FORWARDING(Ret, operator (), Do)

		private:
			template< typename ParamTypeList >
			Ret Do(const Tuple<ParamTypeList>& params) const
			{ return -FunctorInvoker::Invoke(_f, params); }
		};
	}

	template < typename F >
	Detail::NegateFunc<F> negate(const F& f) { return Detail::NegateFunc<F>(f); }


	class NopFunctor : public function_info<void, UnspecifiedParamTypes>
	{
	public:
		STINGRAYKIT_PERFECT_FORWARDING(void, operator (), Do)

	private:
		template< typename ParamTypeList >
		void Do(const Tuple<ParamTypeList>&) const { }
	};


	template <typename T, typename V = T>
	struct Assigner : public function_info<void(T&, const V&)>
	{
		void operator () (T& t, const V& v) const { t = v; }
	};


	template < typename T >
	class Identity : public function_info<T ()>
	{
	private:
		T _value;

	public:
		explicit Identity(const T& value) : _value(value) { }

		STINGRAYKIT_PERFECT_FORWARDING(T, operator (), Do)

	private:
		template < typename Params_ >
		T Do(const Tuple<Params_>&) const { return _value; }
	};

	template < typename T >
	Identity<T> make_identity(const T& value) { return Identity<T>(value); }


	template < typename FuncType >
	class Invoker : public function_info<typename FuncType::RetType, UnspecifiedParamTypes>
	{
	private:
		FuncType	_func;

	public:
		Invoker(const FuncType& func) : _func(func) { }

		template < typename TupleType >
		typename FuncType::RetType operator() (const TupleType& params) const
		{ return FunctorInvoker::Invoke(_func, params); }
	};

	template < typename FuncType >
	Invoker<FuncType> make_invoker(const FuncType& func)
	{ return Invoker<FuncType>(func); }

	/** @} */

}


#endif
