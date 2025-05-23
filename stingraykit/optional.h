#ifndef STINGRAYKIT_OPTIONAL_H
#define STINGRAYKIT_OPTIONAL_H

// Copyright (c) 2011 - 2025, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/compare/comparers.h>
#include <stingraykit/core/InPlaceType.h>
#include <stingraykit/function/FunctorInvoker.h>
#include <stingraykit/metaprogramming/ParamPassingType.h>

namespace stingray
{

	template < typename T >
	class optional;


	namespace Detail
	{
		template < typename To, typename From >
		struct IsConstructibleFromOptional
			:	integral_constant<bool,
					IsConvertible<optional<From>&, To>::Value ||
					IsConvertible<optional<From>&&, To>::Value ||
					IsConvertible<const optional<From>&, To>::Value ||
					IsConvertible<const optional<From>&&, To>::Value ||
					IsConstructible<To, optional<From>&>::Value ||
					IsConstructible<To, optional<From>&&>::Value ||
					IsConstructible<To, const optional<From>&>::Value ||
					IsConstructible<To, const optional<From>&&>::Value>
		{ };

		template < typename To, typename From >
		struct IsAssignableFromOptional
			:	integral_constant<bool,
					IsConstructibleFromOptional<To, From>::Value ||
					IsAssignable<To&, optional<From>&>::Value ||
					IsAssignable<To&, optional<From>&&>::Value ||
					IsAssignable<To&, const optional<From>&>::Value ||
					IsAssignable<To&, const optional<From>&&>::Value>
		{ };
	}


#define DETAIL_OPTIONAL_COPY_CTOR(OtherType_) \
		optional(OtherType_ other) : _value(), _initialized(other.is_initialized()) \
		{ \
			if (other.is_initialized()) \
				_value.Ctor(other.get()); \
		}

#define DETAIL_OPTIONAL_MOVE_CTOR(OtherType_) \
		optional(OtherType_ other) : _value(), _initialized(other.is_initialized()) \
		{ \
			if (other.is_initialized()) \
				_value.Ctor(std::move(other.get())); \
		}

#define DETAIL_OPTIONAL_COPY_ASSIGN(OtherType_) \
		void assign(OtherType_ other) \
		{ \
			if (other.is_initialized()) \
				assign(other.get()); \
			else \
				reset(); \
		}

#define DETAIL_OPTIONAL_MOVE_ASSIGN(OtherType_) \
		void assign(OtherType_ other) \
		{ \
			if (other.is_initialized()) \
				assign(std::move(other.get())); \
			else \
				reset(); \
		}


	template < typename T >
	struct IsOptional;


	template < typename T >
	class optional
	{
		static_assert(!IsSame<typename Decay<T>::ValueT, NullPtrType>::Value, "T can't be NullPtrType");
		static_assert(!IsSame<typename Decay<T>::ValueT, InPlaceType>::Value, "T can't be InPlaceType");
		static_assert(!IsReference<T>::Value, "T can't be reference");

	public:
		using ValueT = T;

	private:
		StorageFor<T>		_value;
		bool				_initialized;

	public:
		using ParamType = T&;
		using ConstParamType = const T&;
		using MoveParamType = T&&;
		using ConstMoveParamType = const T&&;
		using PtrParamType = T*;
		using ConstPtrParamType = const T*;

	public:
		optional() : _value(), _initialized(false)
		{ }

		optional(NullPtrType) : _value(), _initialized(false)
		{ }

		template < typename U = T, typename EnableIf<
				!IsSame<typename Decay<U>::ValueT, optional>::Value && !IsSame<typename Decay<U>::ValueT, InPlaceType>::Value &&
				IsConstructible<T, U>::Value && IsConvertible<U, T>::Value, int>::ValueT = 0 >
		optional(U&& value) : _value(), _initialized(true)
		{ _value.Ctor(std::forward<U>(value)); }

		template < typename U = T, typename EnableIf<
				!IsSame<typename Decay<U>::ValueT, optional>::Value && !IsSame<typename Decay<U>::ValueT, InPlaceType>::Value &&
				IsConstructible<T, U>::Value && !IsConvertible<U, T>::Value, int>::ValueT = 0 >
		explicit optional(U&& value) : _value(), _initialized(true)
		{ _value.Ctor(std::forward<U>(value)); }

		template < typename... Us,
			   typename EnableIf<IsConstructible<T, Us...>::Value, int>::ValueT = 0 >
		explicit optional(InPlaceType, Us&&... args) : _value(), _initialized(true)
		{ _value.Ctor(std::forward<Us>(args)...); }

		template < typename U, typename... Us,
			   typename EnableIf<IsConstructible<T, std::initializer_list<U>&, Us...>::Value, int>::ValueT = 0 >
		explicit optional(InPlaceType, std::initializer_list<U> il, Us&&... args) : _value(), _initialized(true)
		{ _value.Ctor(il, std::forward<Us>(args)...); }

		DETAIL_OPTIONAL_COPY_CTOR(const optional&)
		DETAIL_OPTIONAL_MOVE_CTOR(optional&&)

		template < typename U, typename EnableIf<
				!Detail::IsConstructibleFromOptional<T, U>::Value &&
				IsConstructible<T, const U&>::Value && IsConvertible<const U&, T>::Value, int>::ValueT = 0 >
		DETAIL_OPTIONAL_COPY_CTOR(const optional<U>&)

		template < typename U, typename EnableIf<
				!Detail::IsConstructibleFromOptional<T, U>::Value &&
				IsConstructible<T, const U&>::Value && !IsConvertible<const U&, T>::Value, int>::ValueT = 0 >
		explicit DETAIL_OPTIONAL_COPY_CTOR(const optional<U>&)

		template < typename U, typename EnableIf<
				!Detail::IsConstructibleFromOptional<T, U>::Value &&
				IsConstructible<T, U>::Value && IsConvertible<U, T>::Value, int>::ValueT = 0 >
		DETAIL_OPTIONAL_MOVE_CTOR(optional<U>&&)

		template < typename U, typename EnableIf<
				!Detail::IsConstructibleFromOptional<T, U>::Value &&
				IsConstructible<T, U>::Value && !IsConvertible<U, T>::Value, int>::ValueT = 0 >
		explicit DETAIL_OPTIONAL_MOVE_CTOR(optional<U>&&)

		~optional()										{ reset(); }

		optional& operator = (NullPtrType)				{ reset();					return *this; }

		template < typename U = T, typename EnableIf<
				!IsSame<typename Decay<U>::ValueT, optional>::Value &&
				IsConstructible<T, U>::Value && IsAssignable<T&, U>::Value, int>::ValueT = 0 >
		optional& operator = (U&& value)				{ assign(std::forward<U>(value)); return *this; }

		optional& operator = (const optional& other)	{ assign(other);			return *this; }
		optional& operator = (optional&& other)			{ assign(std::move(other));	return *this; }

		template < typename U, typename EnableIf<
				!Detail::IsAssignableFromOptional<T, U>::Value &&
				IsConstructible<T, const U&>::Value && IsAssignable<T&, const U&>::Value, int>::ValueT = 0 >
		optional& operator = (const optional<U>& other)	{ assign(other);			return *this; }

		template < typename U, typename EnableIf<
				!Detail::IsAssignableFromOptional<T, U>::Value &&
				IsConstructible<T, U>::Value && IsAssignable<T&, U>::Value, int>::ValueT = 0 >
		optional& operator = (optional<U>&& other)		{ assign(std::move(other));	return *this; }

		bool is_initialized() const						{ return _initialized; }
		explicit operator bool () const					{ return is_initialized(); }

		ConstParamType get() const &					{ CheckInitialized(); return _value.Ref(); }
		ConstMoveParamType get() const &&				{ CheckInitialized(); return std::move(_value.Ref()); }
		ParamType get() &								{ CheckInitialized(); return _value.Ref(); }
		MoveParamType get() &&							{ CheckInitialized(); return std::move(_value.Ref()); }

		ConstPtrParamType get_ptr() const				{ CheckInitialized(); return &_value.Ref(); }
		PtrParamType get_ptr()							{ CheckInitialized(); return &_value.Ref(); }

		ConstPtrParamType operator -> () const			{ return get_ptr(); }
		PtrParamType operator -> ()						{ return get_ptr(); }

		ConstParamType operator * () const &			{ return get(); }
		ConstMoveParamType operator * () const &&		{ return std::move(*this).get(); }
		ParamType operator * () &						{ return get(); }
		MoveParamType operator * () &&					{ return std::move(*this).get(); }

		template < typename U >
		T get_value_or(U&& value) const &
		{ return is_initialized() ? _value.Ref() : static_cast<T>(std::forward<U>(value)); }

		template < typename U >
		T get_value_or(U&& value) &&
		{ return is_initialized() ? std::move(_value.Ref()) : static_cast<T>(std::forward<U>(value)); }

		template < typename Functor >
		typename Decay<typename function_info<typename Decay<Functor>::ValueT>::RetType>::ValueT and_then(Functor&& functor) const &
		{
			using RetType = typename Decay<typename function_info<typename Decay<Functor>::ValueT>::RetType>::ValueT;

			static_assert(IsOptional<RetType>::Value, "Expected function that returns an optional<U>");

			if (is_initialized())
				return FunctorInvoker::InvokeArgs(std::forward<Functor>(functor), get());
			else
				return RetType();
		}

		template < typename Functor >
		typename Decay<typename function_info<typename Decay<Functor>::ValueT>::RetType>::ValueT and_then(Functor&& functor) const &&
		{
			using RetType = typename Decay<typename function_info<typename Decay<Functor>::ValueT>::RetType>::ValueT;

			static_assert(IsOptional<RetType>::Value, "Expected function that returns an optional<U>");

			if (is_initialized())
				return FunctorInvoker::InvokeArgs(std::forward<Functor>(functor), std::move(get()));
			else
				return RetType();
		}

		template < typename Functor >
		typename Decay<typename function_info<typename Decay<Functor>::ValueT>::RetType>::ValueT and_then(Functor&& functor) &
		{
			using RetType = typename Decay<typename function_info<typename Decay<Functor>::ValueT>::RetType>::ValueT;

			static_assert(IsOptional<RetType>::Value, "Expected function that returns an optional<U>");

			if (is_initialized())
				return FunctorInvoker::InvokeArgs(std::forward<Functor>(functor), get());
			else
				return RetType();
		}

		template < typename Functor >
		typename Decay<typename function_info<typename Decay<Functor>::ValueT>::RetType>::ValueT and_then(Functor&& functor) &&
		{
			using RetType = typename Decay<typename function_info<typename Decay<Functor>::ValueT>::RetType>::ValueT;

			static_assert(IsOptional<RetType>::Value, "Expected function that returns an optional<U>");

			if (is_initialized())
				return FunctorInvoker::InvokeArgs(std::forward<Functor>(functor), std::move(get()));
			else
				return RetType();
		}

		template < typename Functor >
		optional<typename Decay<typename function_info<typename Decay<Functor>::ValueT>::RetType>::ValueT> transform(Functor&& functor) const &
		{
			using RetType = typename Decay<typename function_info<typename Decay<Functor>::ValueT>::RetType>::ValueT;

			static_assert(!IsOptional<RetType>::Value && !IsSame<RetType, InPlaceType>::Value && !IsSame<RetType, NullPtrType>::Value,
					"Expected function that returns not an optional, InPlace or null");

			if (is_initialized())
				return optional<RetType>(FunctorInvoker::InvokeArgs(std::forward<Functor>(functor), get()));
			else
				return optional<RetType>();
		}

		template < typename Functor >
		optional<typename Decay<typename function_info<typename Decay<Functor>::ValueT>::RetType>::ValueT> transform(Functor&& functor) const &&
		{
			using RetType = typename Decay<typename function_info<typename Decay<Functor>::ValueT>::RetType>::ValueT;

			static_assert(!IsOptional<RetType>::Value && !IsSame<RetType, InPlaceType>::Value && !IsSame<RetType, NullPtrType>::Value,
					"Expected function that returns not an optional, InPlace or null");

			if (is_initialized())
				return optional<RetType>(FunctorInvoker::InvokeArgs(std::forward<Functor>(functor), std::move(get())));
			else
				return optional<RetType>();
		}

		template < typename Functor >
		optional<typename Decay<typename function_info<typename Decay<Functor>::ValueT>::RetType>::ValueT> transform(Functor&& functor) &
		{
			using RetType = typename Decay<typename function_info<typename Decay<Functor>::ValueT>::RetType>::ValueT;

			static_assert(!IsOptional<RetType>::Value && !IsSame<RetType, InPlaceType>::Value && !IsSame<RetType, NullPtrType>::Value,
					"Expected function that returns not an optional, InPlace or null");

			if (is_initialized())
				return optional<RetType>(FunctorInvoker::InvokeArgs(std::forward<Functor>(functor), get()));
			else
				return optional<RetType>();
		}

		template < typename Functor >
		optional<typename Decay<typename function_info<typename Decay<Functor>::ValueT>::RetType>::ValueT> transform(Functor&& functor) &&
		{
			using RetType = typename Decay<typename function_info<typename Decay<Functor>::ValueT>::RetType>::ValueT;

			static_assert(!IsOptional<RetType>::Value && !IsSame<RetType, InPlaceType>::Value && !IsSame<RetType, NullPtrType>::Value,
					"Expected function that returns not an optional, InPlace or null");

			if (is_initialized())
				return optional<RetType>(FunctorInvoker::InvokeArgs(std::forward<Functor>(functor), std::move(get())));
			else
				return optional<RetType>();
		}

		template < typename U = T, typename Functor, typename EnableIf<IsCopyConstructible<U>::Value, int>::ValueT = 0 >
		optional or_else(Functor&& functor) const &
		{
			using RetType = typename Decay<typename function_info<typename Decay<Functor>::ValueT>::RetType>::ValueT;

			static_assert(IsSame<RetType, optional>::Value, "Expected function that returns an optional<T>");

			if (is_initialized())
				return *this;
			else
				return FunctorInvoker::InvokeArgs(std::forward<Functor>(functor));
		}

		template < typename U = T, typename Functor, typename EnableIf<IsMoveConstructible<U>::Value, int>::ValueT = 0 >
		optional or_else(Functor&& functor) &&
		{
			using RetType = typename Decay<typename function_info<typename Decay<Functor>::ValueT>::RetType>::ValueT;

			static_assert(IsSame<RetType, optional>::Value, "Expected function that returns an optional<T>");

			if (is_initialized())
				return std::move(*this);
			else
				return FunctorInvoker::InvokeArgs(std::forward<Functor>(functor));
		}

		int Compare(NullPtrType) const
		{ return is_initialized() ? 1 : 0; }

		int Compare(const T& other) const
		{ return is_initialized() ? comparers::Cmp()(get(), other) : -1; }

		int Compare(const optional& other) const
		{ return other.is_initialized() ? Compare(other.get()) : Compare(null); }

		void swap(optional& other)
		{
			using std::swap;

			if (is_initialized())
			{
				if (other.is_initialized())
					swap(get(), other.get());
				else
				{
					other.assign(std::move(*this).get());
					reset();
				}
			}
			else if (other.is_initialized())
			{
				assign(std::move(other).get());
				other.reset();
			}
		}

		void reset()
		{
			if (_initialized)
			{
				_initialized = false;
				_value.Dtor();
			}
		}

		template < typename U = T, typename EnableIf<
				!IsSame<typename Decay<U>::ValueT, optional>::Value &&
				IsConstructible<T, U>::Value && IsAssignable<T&, U>::Value, int>::ValueT = 0 >
		void assign(U&& value)
		{
			if (_initialized)
				_value.Ref() = std::forward<U>(value);
			else
			{
				_value.Ctor(std::forward<U>(value));
				_initialized = true;
			}
		}

		template < typename U = T, typename EnableIf<
				IsSame<U, T>::Value &&
				IsAssignable<T&, const U&>::Value, int>::ValueT = 0 >
		DETAIL_OPTIONAL_COPY_ASSIGN(const optional&)

		template < typename U = T, typename EnableIf<
				IsSame<U, T>::Value &&
				IsAssignable<T&, U>::Value, int>::ValueT = 0 >
		DETAIL_OPTIONAL_MOVE_ASSIGN(optional&&)

		template < typename U, typename EnableIf<
				!Detail::IsAssignableFromOptional<T, U>::Value &&
				IsConstructible<T, const U&>::Value && IsAssignable<T&, const U&>::Value, int>::ValueT = 0 >
		DETAIL_OPTIONAL_COPY_ASSIGN(const optional<U>&)

		template < typename U, typename EnableIf<
				!Detail::IsAssignableFromOptional<T, U>::Value &&
				IsConstructible<T, U>::Value && IsAssignable<T&, U>::Value, int>::ValueT = 0 >
		DETAIL_OPTIONAL_MOVE_ASSIGN(optional<U>&&)

		template < typename... Us,
				typename EnableIf<IsConstructible<T, Us...>::Value, int>::ValueT = 0 >
		void emplace(Us&&... args)
		{
			reset();
			_value.Ctor(std::forward<Us>(args)...);
			_initialized = true;
		}

		template < typename U, typename... Us,
				typename EnableIf<IsConstructible<T, std::initializer_list<U>&, Us...>::Value, int>::ValueT = 0 >
		void emplace(std::initializer_list<U> il, Us&&... args)
		{
			reset();
			_value.Ctor(il, std::forward<Us>(args)...);
			_initialized = true;
		}

	private:
		void CheckInitialized() const				{ STINGRAYKIT_CHECK(is_initialized(), NotInitializedException()); }
	};


#undef DETAIL_OPTIONAL_COPY_CTOR
#undef DETAIL_OPTIONAL_MOVE_CTOR
#undef DETAIL_OPTIONAL_COPY_ASSIGN
#undef DETAIL_OPTIONAL_MOVE_ASSIGN


	template < typename T >
	bool operator == (const optional<T>& lhs, NullPtrType)
	{ return !lhs.is_initialized(); }
	STINGRAYKIT_GENERATE_NON_MEMBER_COMMUTATIVE_EQUALITY_OPERATORS_FROM_EQUAL(MK_PARAM(template < typename T >), optional<T>, NullPtrType);


	template < typename T >
	bool operator < (const optional<T>& lhs, NullPtrType)
	{ return false; }
	template < typename T >
	bool operator < (NullPtrType, const optional<T>& rhs)
	{ return rhs.is_initialized(); }
	STINGRAYKIT_GENERATE_NON_MEMBER_RELATIONAL_OPERATORS_FROM_LESS(MK_PARAM(template < typename T >), optional<T>, NullPtrType);
	STINGRAYKIT_GENERATE_NON_MEMBER_RELATIONAL_OPERATORS_FROM_LESS(MK_PARAM(template < typename T >), NullPtrType, optional<T>);


	template < typename T, typename U >
	bool operator == (const optional<T>& lhs, const U& rhs)
	{ return lhs.is_initialized() && lhs.get() == rhs; }
	STINGRAYKIT_GENERATE_NON_MEMBER_COMMUTATIVE_EQUALITY_OPERATORS_FROM_EQUAL(MK_PARAM(template < typename T, typename U >), optional<T>, U);


	template < typename T, typename U >
	bool operator < (const optional<T>& lhs, const U& rhs)
	{ return !lhs.is_initialized() || lhs.get() < rhs; }
	template < typename T, typename U >
	bool operator < (const T& lhs, const optional<U>& rhs)
	{ return rhs.is_initialized() && lhs < rhs.get(); }
	STINGRAYKIT_GENERATE_NON_MEMBER_RELATIONAL_OPERATORS_FROM_LESS(MK_PARAM(template < typename T, typename U >), optional<T>, U);
	STINGRAYKIT_GENERATE_NON_MEMBER_RELATIONAL_OPERATORS_FROM_LESS(MK_PARAM(template < typename T, typename U >), T, optional<U>);


	template < typename T, typename U >
	bool operator == (const optional<T>& lhs, const optional<U>& rhs)
	{ return lhs.is_initialized() ? rhs.is_initialized() && lhs.get() == rhs.get() : !rhs.is_initialized(); }
	STINGRAYKIT_GENERATE_NON_MEMBER_EQUALITY_OPERATORS_FROM_EQUAL(MK_PARAM(template < typename T, typename U >), optional<T>, optional<U>);


	template < typename T, typename U >
	bool operator < (const optional<T>& lhs, const optional<U>& rhs)
	{ return rhs.is_initialized() && (!lhs.is_initialized() || (lhs.get() < rhs.get())); }
	STINGRAYKIT_GENERATE_NON_MEMBER_RELATIONAL_OPERATORS_FROM_LESS(MK_PARAM(template < typename T, typename U >), optional<T>, optional<U>);


	template < typename T >
	void swap(optional<T>& lhs, optional<T>& rhs)
	{ lhs.swap(rhs); }


	template < typename T >
	optional<typename Decay<T>::ValueT> make_optional_value(T&& value)
	{ return optional<typename Decay<T>::ValueT>(std::forward<T>(value)); }


	template < typename T, typename... Us >
	optional<T> make_optional_value(Us&&... args)
	{ return optional<T>(InPlace, std::forward<Us>(args)...); }


	template < typename T, typename U, typename... Us >
	optional<T> make_optional_value(std::initializer_list<U> il, Us&&... args)
	{ return optional<T>(InPlace, il, std::forward<Us>(args)...); }


	template < typename CompareFunc >
	class OptionalCmp : public comparers::CmpComparerInfo
	{
		static_assert(comparers::IsCmpComparer<CompareFunc>::Value, "Expected Cmp comparer");

	private:
		const CompareFunc	_compareFunc;

	public:
		explicit OptionalCmp(const CompareFunc& compareFunc = CompareFunc()) : _compareFunc(compareFunc) { }

		template < typename T >
		int operator () (const optional<T>& lhs, NullPtrType) const
		{ return lhs ? 1 : 0; }

		template < typename T >
		int operator () (NullPtrType, const optional<T>& rhs) const
		{ return -(*this)(rhs, null); }

		template < typename T >
		int operator () (const optional<T>& lhs, const T& rhs) const
		{ return lhs ? _compareFunc(*lhs, rhs) : -1; }

		template < typename T >
		int operator () (const T& lhs, const optional<T>& rhs) const
		{ return -(*this)(rhs, lhs); }

		template < typename T >
		int operator () (const optional<T>& lhs, const optional<T>& rhs) const
		{ return rhs ? (*this)(lhs, *rhs) : (*this)(lhs, null); }
	};
	STINGRAYKIT_DECLARE_TEMPLATE_COMPARERS(Optional, typename CompareFunc, CompareFunc);


	template < typename CompareFunc >
	OptionalCmp<CompareFunc> MakeOptionalCmp(const CompareFunc& compareFunc)
	{ return OptionalCmp<CompareFunc>(compareFunc); }


	template < typename T >
	struct IsOptional : FalseType { };

	template < typename T >
	struct IsOptional<optional<T>> : TrueType { };


	template < typename T >
	struct IsNullable<optional<T>> : TrueType { };


	namespace Detail
	{
		template < typename T >
		struct PassByRef<T, typename EnableIf<IsOptional<T>::Value, void>::ValueT> : integral_constant<bool, PassByRef<typename T::ValueT>::Value> { };
	}

}

#endif
