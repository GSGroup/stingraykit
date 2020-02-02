#ifndef STINGRAYKIT_FUNCTION_FUNCTIONAL_H
#define STINGRAYKIT_FUNCTION_FUNCTIONAL_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/KeyValuePair.h>
#include <stingraykit/function/function_info.h>
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
			template < typename ParamTypeList >
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
			template < typename ParamTypeList >
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
		template < typename ParamTypeList >
		void Do(const Tuple<ParamTypeList>&) const { }
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
			T& operator () (const V& v) const { return _ref = v; }
		};
	}

	template < typename T >
	Detail::Assigner<T> make_assigner(T& ref) { return Detail::Assigner<T>(ref); }


	namespace Detail
	{
		template < typename T >
		class Identity : public function_info<T, UnspecifiedParamTypes>
		{
		private:
			T	_value;

		public:
			explicit Identity(const T& value) : _value(value) { }

			STINGRAYKIT_PERFECT_FORWARDING(T, operator (), Do)

		private:
			template < typename Params_ >
			T Do(const Tuple<Params_>&) const { return _value; }
		};
	}

	template < typename T >
	Detail::Identity<T> make_identity(const T& value) { return Detail::Identity<T>(value); }


#define DETAIL_STINGRAY_MAKE_INSTANCE(N_, UserArg_) \
	STINGRAYKIT_INSERT_IF(N_, template<STINGRAYKIT_REPEAT(N_, STINGRAYKIT_TEMPLATE_PARAM_DECL, T)>) \
	Result_ operator()(STINGRAYKIT_REPEAT(N_, STINGRAYKIT_FUNCTION_PARAM_DECL, T)) const \
	{ return Result_(STINGRAYKIT_REPEAT(N_, STINGRAYKIT_FUNCTION_PARAM_USAGE, ~)); }

	template < typename Result_ >
	struct MakeInstance : public function_info<Result_, UnspecifiedParamTypes>
	{
		STINGRAYKIT_REPEAT_NESTING_2(10, DETAIL_STINGRAY_MAKE_INSTANCE, ~)
	};

#undef DETAIL_STINGRAY_MAKE_INSTANCE


#define DETAIL_MAKE_INSTANCE_TYPENAMES(Index_, UserArg_) STINGRAYKIT_COMMA_IF(Index_) typename STINGRAYKIT_CAT(Param, Index_)
#define DETAIL_MAKE_INSTANCE_PARAMDECLS(Index_, UserArg_) STINGRAYKIT_COMMA_IF(Index_) const STINGRAYKIT_CAT(Param, Index_)& STINGRAYKIT_CAT(p, Index_)
#define DETAIL_MAKE_INSTANCE_PARAMS(Index_, UserArg_) STINGRAYKIT_COMMA_IF(Index_) STINGRAYKIT_CAT(p, Index_)
#define DETAIL_DECL_MAKE_INSTANCE(N_, UserArg_) \
	template< typename T STINGRAYKIT_COMMA_IF(N_) STINGRAYKIT_REPEAT(N_, DETAIL_MAKE_INSTANCE_TYPENAMES, STINGRAYKIT_EMPTY()) > \
	T make_instance(STINGRAYKIT_REPEAT(N_, DETAIL_MAKE_INSTANCE_PARAMDECLS, STINGRAYKIT_EMPTY())) \
	{ return T(STINGRAYKIT_REPEAT(N_, DETAIL_MAKE_INSTANCE_PARAMS, STINGRAYKIT_EMPTY())); }

	STINGRAYKIT_REPEAT_NESTING_2(10, DETAIL_DECL_MAKE_INSTANCE, STINGRAYKIT_EMPTY())

#undef DETAIL_MAKE_INSTANCE_PARAMDECLS
#undef DETAIL_MAKE_INSTANCE_TYPENAMES
#undef DETAIL_DECL_MAKE_INSTANCE


	namespace Detail
	{
		template < typename FuncType >
		class Invoker : public function_info<typename FuncType::RetType, UnspecifiedParamTypes>
		{
		private:
			FuncType	_func;

		public:
			Invoker(const FuncType& func) : _func(func) { }

			template < typename Params >
			typename FuncType::RetType operator () (const Tuple<Params>& params) const
			{ return FunctorInvoker::Invoke(_func, params); }

			template < typename Key, typename Value >
			typename FuncType::RetType operator () (const std::pair<Key, Value>& pair) const
			{ return _func(pair.first, pair.second); }

			template < typename Key, typename Value >
			typename FuncType::RetType operator () (const KeyValuePair<Key, Value>& pair) const
			{ return _func(pair.Key, pair.Value); }
		};
	}

	template < typename FuncType >
	Detail::Invoker<FuncType> make_invoker(const FuncType& func) { return Detail::Invoker<FuncType>(func); }

	/** @} */

}


#endif
