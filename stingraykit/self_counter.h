#ifndef STINGRAYKIT_SELF_COUNTER_H
#define STINGRAYKIT_SELF_COUNTER_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/core/NonCopyable.h>
#include <stingraykit/thread/atomic/AtomicInt.h>
#include <stingraykit/CheckedDelete.h>
#include <stingraykit/Dummy.h>
#include <stingraykit/exception.h>
#include <stingraykit/safe_bool.h>

#define STINGRAYKIT_DECLARE_SELF_COUNT_PTR(type) typedef stingray::self_count_ptr< type > type##SelfCountPtr;

namespace stingray
{

	namespace Detail
	{
		struct SelfCounterHelper
		{
			static void CheckAddRef(int count);
			static void CheckReleaseRef(int count);
		};
	}


	struct static_cast_tag
	{ };


	template < typename T >
	class self_count_ptr : public safe_bool<self_count_ptr<T> >
	{
		template < typename U >
		friend class self_count_ptr;

	public:
		typedef T ValueType;

	private:
		T*				_rawPtr;

	public:
		explicit self_count_ptr(T* rawPtr = 0)
			: _rawPtr(rawPtr)
		{ }

		self_count_ptr(const NullPtrType&)
			: _rawPtr()
		{ }

		self_count_ptr(const self_count_ptr<T>& other)
			: _rawPtr(other._rawPtr)
		{
			if (_rawPtr)
				_rawPtr->add_ref();
		}

		template < typename U >
		self_count_ptr(const self_count_ptr<U>& other, typename EnableIf<IsInherited<U, T>::Value, Dummy>::ValueT* = 0)
			: _rawPtr(other._rawPtr)
		{
			if (_rawPtr)
				_rawPtr->add_ref();
		}

		template < typename U >
		self_count_ptr(const self_count_ptr<U>& other, static_cast_tag)
			: _rawPtr(static_cast<T*>(other._rawPtr))
		{
			if (_rawPtr)
				_rawPtr->add_ref();
		}

		~self_count_ptr()
		{
			if (_rawPtr)
				_rawPtr->release_ref();
		}

		self_count_ptr<T>& operator = (const self_count_ptr<T>& other)
		{
			self_count_ptr<T> tmp(other);
			swap(tmp);
			return *this;
		}

		bool operator == (T* ptr) const								{ return _rawPtr == ptr; }
		bool operator != (T* ptr) const								{ return !(*this == ptr); }
		bool operator == (const self_count_ptr<T>& other) const		{ return other == _rawPtr; }
		bool operator != (const self_count_ptr<T>& other) const		{ return !(*this == other); }

		bool is_initialized() const									{ return _rawPtr != 0; }
		bool boolean_test() const									{ return is_initialized(); }

		void reset(T* ptr = 0)
		{
			self_count_ptr<T> tmp(ptr);
			swap(tmp);
		}

		void swap(self_count_ptr<T>& other) { std::swap(_rawPtr, other._rawPtr); }

		T* get() const					{ return _rawPtr; }
		T* operator -> () const			{ check_ptr(); return _rawPtr; }
		T& operator * () const			{ check_ptr(); return *_rawPtr; }
		bool unique() const				{ return _rawPtr ? _rawPtr->value() == 1 : true; }

		bool owner_before(const self_count_ptr& other) const
		{ return get() < other.get(); }

	private:
		void check_ptr() const
		{ STINGRAYKIT_CHECK(_rawPtr, NullPointerException()); }
	};


	template < typename T >
	class self_counter : private NonCopyable
	{
	private:
		mutable AtomicS32::Type	_value;

	protected:
		~self_counter()
		{ }

	public:
		self_counter() : _value(1)
		{ }

		self_count_ptr<T> self_count_ptr_from_this()
		{
			add_ref();
			return self_count_ptr<T>(static_cast<T*>(this));
		}

		void add_ref() const
		{
			const s32 count = AtomicS32::Inc(_value); (void)count;
			STINGRAYKIT_DEBUG_ONLY(Detail::SelfCounterHelper::CheckAddRef(count));
		}

		void release_ref() const
		{
			const s32 count = AtomicS32::Dec(_value);
			if (count == 0)
			{
				STINGRAYKIT_ANNOTATE_HAPPENS_AFTER(&_value);
				STINGRAYKIT_ANNOTATE_RELEASE(&_value);
				CheckedDelete(static_cast<const T*>(this));
			}
			else
				STINGRAYKIT_ANNOTATE_HAPPENS_BEFORE(&_value);
			STINGRAYKIT_DEBUG_ONLY(Detail::SelfCounterHelper::CheckReleaseRef(count));
		}

		int value() const
		{ return _value; }
	};


	template < typename T >
	struct ToPointer<self_count_ptr<T> >
	{ typedef T* ValueT; };


	template < typename T >
	T* to_pointer(const self_count_ptr<T>& ptr)
	{ return ptr.get(); }

	template < typename T >
	T* to_pointer(self_count_ptr<T>& ptr)
	{ return ptr.get(); }


#define DETAIL_MAKE_SELF_COUNT_PTR_TYPENAMES(Index_, UserArg_) STINGRAYKIT_COMMA_IF(Index_) typename STINGRAYKIT_CAT(Param, Index_)
#define DETAIL_MAKE_SELF_COUNT_PTR_PARAMDECLS(Index_, UserArg_) STINGRAYKIT_COMMA_IF(Index_) const STINGRAYKIT_CAT(Param, Index_)& STINGRAYKIT_CAT(p, Index_)
#define DETAIL_MAKE_SELF_COUNT_PTR_PARAMS(Index_, UserArg_) STINGRAYKIT_COMMA_IF(Index_) STINGRAYKIT_CAT(p, Index_)
#define DETAIL_DECL_MAKE_SELF_COUNT_PTR(N_, UserArg_) \
	template< typename T STINGRAYKIT_COMMA_IF(N_) STINGRAYKIT_REPEAT(N_, DETAIL_MAKE_SELF_COUNT_PTR_TYPENAMES, STINGRAYKIT_EMPTY()) > \
	self_count_ptr<T> make_self_count_ptr(STINGRAYKIT_REPEAT(N_, DETAIL_MAKE_SELF_COUNT_PTR_PARAMDECLS, STINGRAYKIT_EMPTY())) \
	{ return self_count_ptr<T>(new T(STINGRAYKIT_REPEAT(N_, DETAIL_MAKE_SELF_COUNT_PTR_PARAMS, STINGRAYKIT_EMPTY()))); }

	STINGRAYKIT_REPEAT_NESTING_2(10, DETAIL_DECL_MAKE_SELF_COUNT_PTR, STINGRAYKIT_EMPTY())


#undef DETAIL_MAKE_SELF_COUNT_PTR_PARAMDECLS
#undef DETAIL_MAKE_SELF_COUNT_PTR_TYPENAMES
#undef DETAIL_DECL_MAKE_SELF_COUNT_PTR


	template < typename T >
	struct IsNullable<self_count_ptr<T> > : public TrueType { };

}

#endif
