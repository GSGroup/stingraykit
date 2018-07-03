#ifndef STINGRAYKIT_COLLECTION_SORTEDOBSERVABLESET_H
#define STINGRAYKIT_COLLECTION_SORTEDOBSERVABLESET_H

// Copyright (c) 2011 - 2018, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/ObservableSetWrapper.h>
#include <stingraykit/collection/SortedSet.h>

namespace stingray
{

	/**
	 * @addtogroup toolkit_collections
	 * @{
	 */

	template <
			typename ValueType_,
			typename CompareType_ = comparers::Less,
			template <class, class, class> class SetType_ = std::set,
			typename AllocatorType_ = std::allocator<ValueType_>
			>
	struct SortedObservableSet
		:	public ObservableSetWrapper<SortedSet<ValueType_, CompareType_, SetType_, AllocatorType_> >
	{
		typedef ObservableSetWrapper<SortedSet<ValueType_, CompareType_, SetType_, AllocatorType_> > base;

		SortedObservableSet() : base() { }
		SortedObservableSet(shared_ptr<IEnumerable<typename base::ValueType> > enumerable) : base(enumerable) { }
		SortedObservableSet(shared_ptr<IEnumerator<typename base::ValueType> > enumerator) : base(enumerator) { }
	};

	template <
			typename T,
			typename CompareType = comparers::Less,
			typename AllocatorType = typename flat_set<T, CompareType>::allocator_type
			>
	struct FlatSortedObservableSet
	{ typedef SortedObservableSet<T, CompareType, flat_set, AllocatorType>		Type; };

	/** @} */

}

#endif
