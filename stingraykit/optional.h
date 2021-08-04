#ifndef STINGRAYKIT_OPTIONAL_H
#define STINGRAYKIT_OPTIONAL_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/compare/comparers.h>
#include <stingraykit/core/InPlaceType.h>

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
		using PtrParamType = T*;
		using ConstPtrParamType = const T*;

	public:
		optional() : _value(), _initialized(false)
		{ }

		optional(NullPtrType) : _value(), _initialized(false)
		{ }

		template < typename U = T, typename EnableIf<
				!IsSame<typename Decay<U>::ValueT, optional>::Value && !IsSame<typename Decay<U>::ValueT, InPlaceType>::Value &&
				IsConstructible<T, U>::Value && IsConvertible<U, T>::Value, bool>::ValueT = true >
		optional(U&& value) : _value(), _initialized(true)
		{ _value.Ctor(std::forward<U>(value)); }

		template < typename U = T, typename EnableIf<
				!IsSame<typename Decay<U>::ValueT, optional>::Value && !IsSame<typename Decay<U>::ValueT, InPlaceType>::Value &&
				IsConstructible<T, U>::Value && !IsConvertible<U, T>::Value, bool>::ValueT = false >
		explicit optional(U&& value) : _value(), _initialized(true)
		{ _value.Ctor(std::forward<U>(value)); }

		template < typename... Us,
			   typename EnableIf<IsConstructible<T, Us...>::Value, bool>::ValueT = true >
		explicit optional(InPlaceType, Us&&... args) : _value(), _initialized(true)
		{ _value.Ctor(std::forward<Us>(args)...); }

		template < typename U, typename... Us,
			   typename EnableIf<IsConstructible<T, std::initializer_list<U>&, Us...>::Value, bool>::ValueT = false >
		explicit optional(InPlaceType, std::initializer_list<U> il, Us&&... args) : _value(), _initialized(true)
		{ _value.Ctor(il, std::forward<Us>(args)...); }

		DETAIL_OPTIONAL_COPY_CTOR(const optional&)
		DETAIL_OPTIONAL_MOVE_CTOR(optional&&)

		template < typename U, typename EnableIf<
				!Detail::IsConstructibleFromOptional<T, U>::Value &&
				IsConstructible<T, const U&>::Value && IsConvertible<const U&, T>::Value, bool>::ValueT = true >
		DETAIL_OPTIONAL_COPY_CTOR(const optional<U>&)

		template < typename U, typename EnableIf<
				!Detail::IsConstructibleFromOptional<T, U>::Value &&
				IsConstructible<T, const U&>::Value && !IsConvertible<const U&, T>::Value, bool>::ValueT = false >
		explicit DETAIL_OPTIONAL_COPY_CTOR(const optional<U>&)

		template < typename U, typename EnableIf<
				!Detail::IsConstructibleFromOptional<T, U>::Value &&
				IsConstructible<T, U>::Value && IsConvertible<U, T>::Value, bool>::ValueT = true >
		DETAIL_OPTIONAL_MOVE_CTOR(optional<U>&&)

		template < typename U, typename EnableIf<
				!Detail::IsConstructibleFromOptional<T, U>::Value &&
				IsConstructible<T, U>::Value && !IsConvertible<U, T>::Value, bool>::ValueT = false >
		explicit DETAIL_OPTIONAL_MOVE_CTOR(optional<U>&&)

		~optional()										{ reset(); }

		optional& operator = (NullPtrType)				{ reset();					return *this; }

		template < typename U = T, typename EnableIf<
				!IsSame<typename Decay<U>::ValueT, optional>::Value &&
				IsConstructible<T, U>::Value && IsAssignable<T&, U>::Value, bool>::ValueT = true >
		optional& operator = (U&& value)				{ assign(std::forward<U>(value)); return *this; }

		optional& operator = (const optional& other)	{ assign(other);			return *this; }
		optional& operator = (optional&& other)			{ assign(std::move(other));	return *this; }

		template < typename U, typename EnableIf<
				!Detail::IsAssignableFromOptional<T, U>::Value &&
				IsConstructible<T, const U&>::Value && IsAssignable<T&, const U&>::Value, bool>::ValueT = true >
		optional& operator = (const optional<U>& other)	{ assign(other);			return *this; }

		template < typename U, typename EnableIf<
				!Detail::IsAssignableFromOptional<T, U>::Value &&
				IsConstructible<T, U>::Value && IsAssignable<T&, U>::Value, bool>::ValueT = false >
		optional& operator = (optional<U>&& other)		{ assign(std::move(other));	return *this; }

		ConstParamType get() const						{ CheckInitialized(); return _value.Ref(); }
		ParamType get()									{ CheckInitialized(); return _value.Ref(); }

		ConstParamType get_value_or(ConstParamType value) const { return is_initialized() ? _value.Ref() : value; }

		ConstPtrParamType get_ptr() const				{ CheckInitialized(); return &_value.Ref(); }
		PtrParamType get_ptr()							{ CheckInitialized(); return &_value.Ref(); }

		ConstPtrParamType operator -> () const			{ return get_ptr(); }
		PtrParamType operator -> ()						{ return get_ptr(); }

		ConstParamType operator * () const				{ return get(); }
		ParamType operator * ()							{ return get(); }

		void reset()
		{
			if (_initialized)
			{
				_initialized = false;
				_value.Dtor();
			}
		}

		bool is_initialized() const						{ return _initialized; }
		explicit operator bool () const					{ return is_initialized(); }

		bool operator == (const optional& rhs) const
		{ return is_initialized() ? rhs.is_initialized() && get() == rhs.get() : !rhs.is_initialized(); }
		STINGRAYKIT_GENERATE_EQUALITY_OPERATORS_FROM_EQUAL(optional);

		bool operator < (const optional& rhs) const
		{ return rhs.is_initialized() && (!is_initialized() || (get() < rhs.get())); }
		STINGRAYKIT_GENERATE_RELATIONAL_OPERATORS_FROM_LESS(optional);

		int Compare(const optional& other) const
		{
			if (is_initialized() && other.is_initialized())
				return comparers::Cmp()(get(), other.get());
			return comparers::Cmp()(is_initialized(), other.is_initialized());
		}

		template < typename U = T, typename EnableIf<
				!IsSame<typename Decay<U>::ValueT, optional>::Value &&
				IsConstructible<T, U>::Value && IsAssignable<T&, U>::Value, bool>::ValueT = true >
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

		DETAIL_OPTIONAL_COPY_ASSIGN(const optional&)
		DETAIL_OPTIONAL_MOVE_ASSIGN(optional&&)

		template < typename U, typename EnableIf<
				!Detail::IsAssignableFromOptional<T, U>::Value &&
				IsConstructible<T, const U&>::Value && IsAssignable<T&, const U&>::Value, bool>::ValueT = true >
		DETAIL_OPTIONAL_COPY_ASSIGN(const optional<U>&)

		template < typename U, typename EnableIf<
				!Detail::IsAssignableFromOptional<T, U>::Value &&
				IsConstructible<T, U>::Value && IsAssignable<T&, U>::Value, bool>::ValueT = false >
		DETAIL_OPTIONAL_MOVE_ASSIGN(optional<U>&&)

		template < typename... Us,
				typename EnableIf<IsConstructible<T, Us...>::Value, bool>::ValueT = true>
		void emplace(Us&&... args)
		{
			reset();
			_value.Ctor(std::forward<Us>(args)...);
			_initialized = true;
		}

		template < typename U, typename... Us,
				typename EnableIf<IsConstructible<T, std::initializer_list<U>&, Us...>::Value, bool>::ValueT = true>
		void emplace(std::initializer_list<U> il, Us&&... args)
		{
			reset();
			_value.Ctor(il, std::forward<Us>(args)...);
			_initialized = true;
		}

	private:
		void CheckInitialized() const				{ STINGRAYKIT_CHECK(is_initialized(), "Not initialized!"); }
	};


#undef DETAIL_OPTIONAL_COPY_CTOR
#undef DETAIL_OPTIONAL_MOVE_CTOR
#undef DETAIL_OPTIONAL_COPY_ASSIGN
#undef DETAIL_OPTIONAL_MOVE_ASSIGN


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
	class OptionalCmp : public function_info<int, UnspecifiedParamTypes>
	{
	private:
		const CompareFunc	_compareFunc;

	public:
		explicit OptionalCmp(const CompareFunc& compareFunc = CompareFunc()) : _compareFunc(compareFunc) { }

		template < typename T >
		int operator() (const optional<T>& lhs, const optional<T>& rhs) const
		{ return (lhs && rhs) ? _compareFunc(*lhs, *rhs) : (lhs ? 1 : (rhs ? -1 : 0)); }
	};


	template < typename CompareFunc >
	OptionalCmp<CompareFunc> MakeOptionalCmp(const CompareFunc& compareFunc)
	{ return OptionalCmp<CompareFunc>(compareFunc); }


	template < typename T >
	struct IsOptional : FalseType { };

	template < typename T >
	struct IsOptional<optional<T>> : TrueType { };


	template < typename T >
	struct IsNullable<optional<T>> : TrueType { };

}

#endif
