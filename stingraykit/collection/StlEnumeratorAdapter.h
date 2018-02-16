#ifndef STINGRAYKIT_COLLECTION_STLENUMERATORADAPTER_H
#define STINGRAYKIT_COLLECTION_STLENUMERATORADAPTER_H

// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
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
	class StlEnumeratorAdapter : public std::iterator< std::forward_iterator_tag, T >
	{
	private:
		shared_ptr<IEnumerator<T> >		_enumerator;

	public:
		StlEnumeratorAdapter() // This means the end of a collection
		{ }

		StlEnumeratorAdapter(const shared_ptr<IEnumerator<T> >& enumerator)
			: _enumerator(enumerator)
		{ }

		const StlEnumeratorAdapter& operator ++ ()
		{
			if (_enumerator->Valid()) // Should we throw if _enumerator is not valid?
				_enumerator->Next();
			return *this;
		}

		T operator * () const
		{
			if (!_enumerator->Valid())
				STINGRAYKIT_THROW(std::runtime_error("Trying to dereference an invalid enumerator!"));
			return _enumerator->Get();
		}

		bool operator != (const StlEnumeratorAdapter& other) const
		{
			if (other._enumerator != NULL)
				STINGRAYKIT_THROW(NotImplementedException());
			return _enumerator->Valid();
		}

		// whatever
	};


	template < typename T >
	inline StlEnumeratorAdapter<T> Wrap(shared_ptr<IEnumerator<T> > enumerator)
	{ return StlEnumeratorAdapter<T>(enumerator); }


	template < typename T >
	inline StlEnumeratorAdapter<T> WrapEnd(shared_ptr<IEnumerator<T> >/* enumerator*/)
	{ return StlEnumeratorAdapter<T>(); }


	template < typename T >
	inline StlEnumeratorAdapter<T> Wrap(shared_ptr<IEnumerable<T> > enumerable)
	{ return StlEnumeratorAdapter<T>(enumerable->GetEnumerator()); }


	template < typename T >
	inline StlEnumeratorAdapter<T> WrapEnd(shared_ptr<IEnumerable<T> >/* enumerator*/)
	{ return StlEnumeratorAdapter<T>(); }

	/** @} */

}

#endif
