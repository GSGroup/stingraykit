#ifndef STINGRAYKIT_COLLECTION_STLENUMERATORADAPTER_H
#define STINGRAYKIT_COLLECTION_STLENUMERATORADAPTER_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/IEnumerator.h>

namespace stingray
{

	/**
	 * @addtogroup toolkit_collections
	 * @{
	 */

	template < typename T >
	class StlEnumeratorAdapter : public std::iterator<std::input_iterator_tag, T>
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


	template < typename T >
	StlEnumeratorAdapter<T> Wrap(const shared_ptr<IEnumerator<T>>& enumerator)
	{ return StlEnumeratorAdapter<T>(enumerator); }


	template < typename T >
	StlEnumeratorAdapter<T> WrapEnd(const shared_ptr<IEnumerator<T>>& /*enumerator*/)
	{ return StlEnumeratorAdapter<T>(); }


	template < typename T >
	StlEnumeratorAdapter<T> Wrap(const shared_ptr<IEnumerable<T>>& enumerable)
	{ return StlEnumeratorAdapter<T>(enumerable->GetEnumerator()); }


	template < typename T >
	StlEnumeratorAdapter<T> WrapEnd(const shared_ptr<IEnumerable<T>>& /*enumerable*/)
	{ return StlEnumeratorAdapter<T>(); }


	namespace Detail
	{

		template < typename T >
		class RangeBasedForEnumeratorAdapter
		{
		private:
			shared_ptr<IEnumerator<T>>	_enumerator;

		public:
			explicit RangeBasedForEnumeratorAdapter(const shared_ptr<IEnumerator<T>>& enumerator) : _enumerator(STINGRAYKIT_REQUIRE_NOT_NULL(enumerator)) { }

			StlEnumeratorAdapter<T> begin() const	{ return Wrap(_enumerator); }
			StlEnumeratorAdapter<T> end() const		{ return WrapEnd(_enumerator); }
		};

	}


	template < typename T >
	Detail::RangeBasedForEnumeratorAdapter<T> IterableEnumerable(const shared_ptr<IEnumerator<T>>& enumerator)
	{ return Detail::RangeBasedForEnumeratorAdapter<T>(enumerator); }


	template < typename T >
	Detail::RangeBasedForEnumeratorAdapter<T> IterableEnumerable(const shared_ptr<IEnumerable<T>>& enumerable)
	{ return Detail::RangeBasedForEnumeratorAdapter<T>(enumerable->GetEnumerator()); }

	/** @} */

}

#endif
