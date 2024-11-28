#ifndef STINGRAYKIT_COLLECTION_STLENUMERATORADAPTER_H
#define STINGRAYKIT_COLLECTION_STLENUMERATORADAPTER_H

// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/IEnumerable.h>

namespace stingray
{

	/**
	 * @addtogroup toolkit_collections
	 * @{
	 */

	template < typename T >
	class StlEnumeratorAdapter : public std::iterator<std::input_iterator_tag, T, ptrdiff_t, const T*, T>
	{
	private:
		shared_ptr<IEnumerator<T>>		_enumerator;

	public:
		StlEnumeratorAdapter() // This means the end of a collection
		{ }

		StlEnumeratorAdapter(const shared_ptr<IEnumerator<T>>& enumerator)
			: _enumerator(STINGRAYKIT_REQUIRE_NOT_NULL(enumerator))
		{ }

		StlEnumeratorAdapter& operator ++ ()
		{
			STINGRAYKIT_CHECK(_enumerator, "Trying to increment an invalid enumerator!");
			_enumerator->Next();
			return *this;
		}

		T operator * () const
		{
			STINGRAYKIT_CHECK(_enumerator, "Trying to dereference an invalid enumerator!");
			return _enumerator->Get();
		}

		bool operator != (const StlEnumeratorAdapter& other) const
		{
			STINGRAYKIT_CHECK(!_enumerator || !other._enumerator, LogicException("Invalid comparison!"));
			return (_enumerator && _enumerator->Valid()) != (other._enumerator && other._enumerator->Valid());
		}

		bool operator == (const StlEnumeratorAdapter& other) const
		{ return !(*this != other); }

		// whatever
	};


	template < typename SrcEnumerator >
	StlEnumeratorAdapter<typename SrcEnumerator::ItemType> Wrap(const shared_ptr<SrcEnumerator>& enumerator, typename EnableIf<IsEnumerator<SrcEnumerator>::Value, int>::ValueT = 0)
	{ return StlEnumeratorAdapter<typename SrcEnumerator::ItemType>(enumerator); }


	template < typename SrcEnumerator >
	StlEnumeratorAdapter<typename SrcEnumerator::ItemType> WrapEnd(const shared_ptr<SrcEnumerator>& /*enumerator*/, typename EnableIf<IsEnumerator<SrcEnumerator>::Value, int>::ValueT = 0)
	{ return StlEnumeratorAdapter<typename SrcEnumerator::ItemType>(); }


	template < typename SrcEnumerable >
	StlEnumeratorAdapter<typename SrcEnumerable::ItemType> Wrap(const shared_ptr<SrcEnumerable>& enumerable, typename EnableIf<IsEnumerable<SrcEnumerable>::Value, int>::ValueT = 0)
	{ return StlEnumeratorAdapter<typename SrcEnumerable::ItemType>(STINGRAYKIT_REQUIRE_NOT_NULL(enumerable)->GetEnumerator()); }


	template < typename SrcEnumerable >
	StlEnumeratorAdapter<typename SrcEnumerable::ItemType> WrapEnd(const shared_ptr<SrcEnumerable>& /*enumerable*/, typename EnableIf<IsEnumerable<SrcEnumerable>::Value, int>::ValueT = 0)
	{ return StlEnumeratorAdapter<typename SrcEnumerable::ItemType>(); }


	namespace Detail
	{

		template < typename T >
		class RangeBasedForEnumeratorAdapter
		{
		public:
			using const_iterator = StlEnumeratorAdapter<T>;
			using value_type = T;

		private:
			shared_ptr<IEnumerator<T>>	_enumerator;

		public:
			explicit RangeBasedForEnumeratorAdapter(const shared_ptr<IEnumerator<T>>& enumerator) : _enumerator(STINGRAYKIT_REQUIRE_NOT_NULL(enumerator)) { }

			const_iterator begin() const	{ return Wrap(_enumerator); }
			const_iterator end() const		{ return WrapEnd(_enumerator); }
		};

	}


	template < typename SrcEnumerator >
	Detail::RangeBasedForEnumeratorAdapter<typename SrcEnumerator::ItemType> IterableEnumerable(const shared_ptr<SrcEnumerator>& enumerator, typename EnableIf<IsEnumerator<SrcEnumerator>::Value, int>::ValueT = 0)
	{ return Detail::RangeBasedForEnumeratorAdapter<typename SrcEnumerator::ItemType>(enumerator); }


	template < typename SrcEnumerable >
	Detail::RangeBasedForEnumeratorAdapter<typename SrcEnumerable::ItemType> IterableEnumerable(const shared_ptr<SrcEnumerable>& enumerable, typename EnableIf<IsEnumerable<SrcEnumerable>::Value, int>::ValueT = 0)
	{ return Detail::RangeBasedForEnumeratorAdapter<typename SrcEnumerable::ItemType>(STINGRAYKIT_REQUIRE_NOT_NULL(enumerable)->GetEnumerator()); }

	/** @} */

}

#endif
