#ifndef STINGRAYKIT_REF_COUNT_H
#define STINGRAYKIT_REF_COUNT_H

// Copyright (c) 2011 - 2025, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/core/NonCopyable.h>
#include <stingraykit/core/NullPtrType.h>
#include <stingraykit/thread/atomic/AtomicInt.h>
#include <stingraykit/CheckedDelete.h>
#include <stingraykit/MetaProgramming.h>

#include <assert.h>

namespace stingray
{

	template < typename UserDataType >
	class basic_ref_count_data
	{
		STINGRAYKIT_NONCOPYABLE(basic_ref_count_data);

	public:
		AtomicU32::Type		Value;
		AtomicU32::Type		SelfCount;
		UserDataType		UserData;

		inline basic_ref_count_data(const UserDataType& userData)
			: Value(1), SelfCount(1), UserData(userData)
		{ }
	};

	template < >
	class basic_ref_count_data<NullType>
	{
		STINGRAYKIT_NONCOPYABLE(basic_ref_count_data);

	public:
		AtomicU32::Type		Value;
		AtomicU32::Type		SelfCount;

		inline basic_ref_count_data()
			: Value(1), SelfCount(1)
		{ }

		inline basic_ref_count_data(const NullType& userData)
			: Value(1), SelfCount(1)
		{ }
	};


	template < typename UserDataType >
	class basic_ref_count
	{
		typedef basic_ref_count_data<UserDataType>		Data;

		Data		*_value;

	public:
		inline basic_ref_count(NullPtrType) : _value()
		{ }

		inline basic_ref_count() : _value(new Data)
		{ }

		inline basic_ref_count(const UserDataType& userData) :
			_value(new Data(userData))
		{ }

		inline basic_ref_count(const basic_ref_count& other) :
			_value(other._value)
		{ if (_value) add_ref_self(); }

		inline ~basic_ref_count()
		{ if (_value) release_self(); }

		inline basic_ref_count& operator = (const basic_ref_count& other)
		{
			basic_ref_count tmp(other);
			swap(tmp);
			return *this;
		}

		inline const UserDataType& GetUserData() const { assert(_value); return _value->UserData; }
		inline bool IsNull() const { return !_value; }

		inline u32 get() const				{ assert(_value); return _value->Value; }
		inline u32 add_ref()				{ assert(_value); return AtomicU32::Inc(_value->Value); }
		inline u32 release()				{ assert(_value); return AtomicU32::Dec(_value->Value); }
		inline bool release_if_unique()		{ assert(_value); return AtomicU32::CompareAndExchange(_value->Value, 1, 0) == 1; }

		inline void swap(basic_ref_count& other)
		{ std::swap(_value, other._value); }

		const Data* get_ptr() const
		{ return _value; }

	private:
		inline u32 add_ref_self()
		{
			assert(_value);
			u32 result = AtomicU32::Inc(_value->SelfCount);
			assert(result > 0);
			return result;
		}
		inline u32 release_self()
		{
			assert(_value);
			u32 result = AtomicU32::Dec(_value->SelfCount);
			assert(result >= 0);
			if (result == 0)
			{
				STINGRAYKIT_ANNOTATE_HAPPENS_AFTER(_value);
				STINGRAYKIT_ANNOTATE_RELEASE(_value);
				CheckedDelete(_value);
			}
			else
				STINGRAYKIT_ANNOTATE_HAPPENS_BEFORE(_value);
			return result;
		}
	};


	typedef basic_ref_count<NullType>		ref_count;


}



#endif
