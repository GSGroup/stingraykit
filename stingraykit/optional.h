#ifndef STINGRAYKIT_OPTIONAL_H
#define STINGRAYKIT_OPTIONAL_H

// Copyright (c) 2011 - 2015, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/aligned_storage.h>
#include <stingraykit/compare/comparers.h>
#include <stingraykit/exception.h>
#include <stingraykit/safe_bool.h>

namespace stingray
{

	template<typename T>
	struct optional : public safe_bool<optional<T> >
	{
		typedef typename Deconst<typename Dereference<T>::ValueT>::ValueT RawType;

	private:
		StorageFor<RawType>	_value;
		bool				_initialized;

	public:
		typedef RawType&		ParamType;
		typedef const RawType&	ConstParamType;
		typedef RawType*		PtrParamType;
		typedef const RawType*	ConstPtrParamType;


	public:
		optional() : _value(), _initialized(false)
		{ }

		optional(const NullPtrType&) : _value(), _initialized(false)
		{ }

		optional(ConstParamType value): _value(), _initialized(true)
		{ _value.Ctor(value); }

		optional(const optional& other): _value(), _initialized(other.is_initialized())
		{
			if (other.is_initialized())
				_value.Ctor(other.get());
		}

		~optional()									{ reset(); }

		optional& operator=(const NullPtrType&)		{ reset();			return *this; }
		optional& operator=(ConstParamType value)	{ assign(value);	return *this; }
		optional& operator=(const optional& other)	{ assign(other);	return *this; }

		ConstParamType get() const					{ CheckInitialized(); return _value.Ref(); }
		ParamType      get()						{ CheckInitialized(); return _value.Ref(); }

		ConstParamType get_value_or(ConstParamType value) const { return is_initialized()? _value.Ref() : value; }

		ConstPtrParamType get_ptr() const			{ CheckInitialized(); return &_value.Ref(); }
		PtrParamType get_ptr()						{ CheckInitialized(); return &_value.Ref(); }

		ConstPtrParamType operator->() const		{ return get_ptr(); }
		PtrParamType operator->()					{ return get_ptr(); }

		ConstParamType operator*() const			{ return get(); }
		ParamType operator*()						{ return get(); }

		void reset()
		{
			if (_initialized)
			{
				_initialized = false;
				_value.Dtor();
			}
		}

		bool is_initialized() const					{ return _initialized; }
		bool boolean_test() const					{ return is_initialized(); }

		bool operator<(const optional& rhs) const
		{ return rhs.is_initialized() && (!is_initialized() || (get() < rhs.get())); }
		STINGRAYKIT_GENERATE_COMPARISON_OPERATORS_FROM_LESS(optional);

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

		void assign(const optional& other)
		{
			if (other.is_initialized())
				assign(other.get());
			else
				reset();
		}

#define DETAIL_STINGRAYKIT_OPTIONAL_EMPLACE(N_, UserArg_) \
		STINGRAYKIT_INSERT_IF(N_, template<STINGRAYKIT_REPEAT(N_, STINGRAYKIT_TEMPLATE_PARAM_DECL, T)>) \
		void emplace(STINGRAYKIT_REPEAT(N_, STINGRAYKIT_FUNCTION_PARAM_DECL, T)) \
		{ \
			reset(); \
			_value.Ctor(STINGRAYKIT_REPEAT(N_, STINGRAYKIT_FUNCTION_PARAM_USAGE, ~)); \
			_initialized = true; \
		}

		STINGRAYKIT_REPEAT_NESTING_2(10, DETAIL_STINGRAYKIT_OPTIONAL_EMPLACE, ~)

#undef DETAIL_STINGRAYKIT_OPTIONAL_EMPLACE

	private:
		void CheckInitialized() const				{ STINGRAYKIT_CHECK(is_initialized(), "Not initialized!"); }
	};

}


#endif

