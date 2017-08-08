#ifndef STINGRAYKIT_COLLECTION_SORTEDOBSERVABLESET_H
#define STINGRAYKIT_COLLECTION_SORTEDOBSERVABLESET_H

// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/IObservableSet.h>
#include <stingraykit/collection/SortedSet.h>

namespace stingray
{

	/**
	 * @addtogroup toolkit_collections
	 * @{
	 */

	template < typename ValueType_ , typename CompareType_ = comparers::Less >
	struct SortedObservableSet : public ObservableSetWrapper<SortedSet<ValueType_, CompareType_> >
	{
		typedef ObservableSetWrapper<SortedSet<ValueType_, CompareType_> > base;

		SortedObservableSet() : base() { }
		SortedObservableSet(shared_ptr<IEnumerable<typename base::ValueType> > enumerable) : base(enumerable) { }
		SortedObservableSet(shared_ptr<IEnumerator<typename base::ValueType> > enumerator) : base(enumerator) { }
	};

	/** @} */

}


#endif
