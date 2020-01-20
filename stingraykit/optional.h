#ifndef STINGRAYKIT_OPTIONAL_H
#define STINGRAYKIT_OPTIONAL_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/compare/comparers.h>

namespace stingray
{

	template < typename T >
	struct optional
	{
		typedef typename Decay<T>::ValueT RawType;

	private:
		StorageFor<RawType>	_value;
		bool				_initialized;

	public:
		typedef RawType&		ParamType;
		typedef const RawType&	ConstParamType;
		typedef RawType&&		MoveParamType;
		typedef RawType*		PtrParamType;
		typedef const RawType*	ConstPtrParamType;

	public:
		optional() : _value(), _initialized(false)
		{ }

		optional(const NullPtrType&) : _value(), _initialized(false)
		{ }

		optional(ConstParamType value) : _value(), _initialized(true)
		{ _value.Ctor(value); }

		optional(MoveParamType value) : _value(), _initialized(true)
		{ _value.Ctor(std::move(value)); }

		optional(const optional& other) : _value(), _initialized(other.is_initialized())
		{
			if (other.is_initialized())
				_value.Ctor(other.get());
		}

		optional(optional&& other) : _value(), _initialized(other.is_initialized())
		{
			if (other.is_initialized())
				_value.Ctor(std::move(other.get()));
		}

		~optional()										{ reset(); }

		optional& operator = (const NullPtrType&)		{ reset();					return *this; }
		optional& operator = (ConstParamType value)		{ assign(value);			return *this; }
		optional& operator = (MoveParamType value)		{ assign(std::move(value));	return *this; }
		optional& operator = (const optional& other)	{ assign(other);			return *this; }
		optional& operator = (optional&& other)			{ assign(std::move(other));	return *this; }

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

		void assign(ConstParamType value)
		{
			if (_initialized)
				_value.Ref() = value;
			else
			{
				_value.Ctor(value);
				_initialized = true;
			}
		}

		void assign(MoveParamType value)
		{
			if (_initialized)
				_value.Ref() = std::move(value);
			else
			{
				_value.Ctor(std::move(value));
				_initialized = true;
			}
		}

		void assign(const optional& other)
		{
			if (other.is_initialized())
				assign(other.get());
			else
				reset();
		}

		void assign(optional&& other)
		{
			if (other.is_initialized())
				assign(std::move(other.get()));
			else
				reset();
		}

		template < typename... Us >
		void emplace(Us&&... args)
		{
			reset();
			_value.Ctor(std::forward<Us>(args)...);
			_initialized = true;
		}

	private:
		void CheckInitialized() const				{ STINGRAYKIT_CHECK(is_initialized(), "Not initialized!"); }
	};


	template < typename T >
	optional<typename Decay<T>::ValueT> make_optional(T&& t)
	{ return optional<typename Decay<T>::ValueT>(std::forward<T>(t)); }


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
	struct IsOptional<optional<T> > : TrueType { };


	template < typename T >
	struct IsNullable<optional<T> > : TrueType { };

}

#endif
