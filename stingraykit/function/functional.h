#ifndef STINGRAYKIT_FUNCTION_FUNCTIONAL_H
#define STINGRAYKIT_FUNCTION_FUNCTIONAL_H

// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/KeyValuePair.h>
#include <stingraykit/function/FunctorInvoker.h>

namespace stingray
{

	/**
	 * @addtogroup toolkit_functions
	 * @{
	 */

	namespace Detail
	{
		template < typename FuncType >
		class NotFunc : public function_info<bool, typename function_info<typename Decay<FuncType>::ValueT>::ParamTypes>
		{
			typedef typename Decay<FuncType>::ValueT RawFuncType;

		private:
			RawFuncType	_func;

		public:
			NotFunc(FuncType&& func) : _func(std::forward<FuncType>(func))
			{ }

			template < typename... Ts >
			bool operator () (Ts&&... args) const
			{ return !FunctorInvoker::InvokeArgs(_func, std::forward<Ts>(args)...); }
		};
	}

	template < typename FuncType >
	Detail::NotFunc<FuncType> not_(FuncType&& func)
	{ return Detail::NotFunc<FuncType>(std::forward<FuncType>(func)); }


	namespace Detail
	{
		template < typename FuncType >
		class NegateFunc : public function_info<typename Decay<FuncType>::ValueT>
		{
			typedef typename Decay<FuncType>::ValueT RawFuncType;

		public:
			typedef typename function_info<RawFuncType>::RetType RetType;

		private:
			RawFuncType	_func;

		public:
			NegateFunc(FuncType&& func) : _func(std::forward<FuncType>(func))
			{ }

			template < typename... Ts >
			RetType operator () (Ts&&... args) const
			{ return -FunctorInvoker::InvokeArgs(_func, std::forward<Ts>(args)...); }
		};
	}

	template < typename FuncType >
	Detail::NegateFunc<FuncType> negate(FuncType&& func)
	{ return Detail::NegateFunc<FuncType>(std::forward<FuncType>(func)); }


	struct NopFunctor : public function_info<void, UnspecifiedParamTypes>
	{
		template < typename... Ts >
		void operator () (Ts&&... args) const { }
	};


	namespace Detail
	{
		template < typename T >
		class Assigner : public function_info<T&, UnspecifiedParamTypes>
		{
		private:
			T&	_ref;

		public:
			explicit Assigner(T& ref) : _ref(ref) { }

			template < typename V >
			T& operator () (V&& v) const { return _ref = std::forward<V>(v); }
		};
	}

	template < typename T >
	Detail::Assigner<T> make_assigner(T& ref)
	{ return Detail::Assigner<T>(ref); }


	namespace Detail
	{
		template < typename T >
		class Identity : public function_info<T, UnspecifiedParamTypes>
		{
		public:
			typedef typename Decay<T>::ValueT Type;

		private:
			Type	_value;

		public:
			explicit Identity(T&& value) : _value(std::forward<T>(value)) { }

			template < typename... Ts >
			Type operator () (Ts&&...) const
			{ return _value; }
		};
	}

	template < typename T >
	Detail::Identity<T> make_identity(T&& value)
	{ return Detail::Identity<T>(std::forward<T>(value)); }


	template < typename Result_ >
	struct MakeInstance : public function_info<Result_, UnspecifiedParamTypes>
	{
		template < typename... Ts >
		Result_ operator () (Ts&&... args) const
		{ return Result_(std::forward<Ts>(args)...); }
	};


	namespace Detail
	{
		template < typename FuncType >
		class Invoker : public function_info<typename function_info<typename Decay<FuncType>::ValueT>::RetType, UnspecifiedParamTypes>
		{
			typedef typename Decay<FuncType>::ValueT RawFuncType;

		public:
			typedef typename function_info<RawFuncType>::RetType RetType;

		private:
			RawFuncType	_func;

		public:
			Invoker(FuncType&& func) : _func(std::forward<FuncType>(func)) { }

			template < typename Params >
			RetType operator () (const Tuple<Params>& params) const
			{ return FunctorInvoker::Invoke(_func, params); }

			template < typename Key, typename Value >
			RetType operator () (const std::pair<Key, Value>& pair) const
			{ return FunctorInvoker::InvokeArgs(_func, pair.first, pair.second); }

			template < typename Key, typename Value >
			RetType operator () (const KeyValuePair<Key, Value>& pair) const
			{ return FunctorInvoker::InvokeArgs(_func, pair.Key, pair.Value); }
		};
	}

	template < typename FuncType >
	Detail::Invoker<FuncType> make_invoker(FuncType&& func)
	{ return Detail::Invoker<FuncType>(std::forward<FuncType>(func)); }

	/** @} */

}


#endif
