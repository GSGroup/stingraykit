#ifndef STINGRAYKIT_COLLECTION_ARRAYOBSERVABLELIST_H
#define STINGRAYKIT_COLLECTION_ARRAYOBSERVABLELIST_H

// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/ArrayList.h>
#include <stingraykit/collection/IObservableList.h>

namespace stingray
{

	/**
	 * @addtogroup toolkit_collections
	 * @{
	 */

	template < typename ValueType_ >
	struct ArrayObservableList : public ObservableListWrapper<ArrayList<ValueType_> >
	{
		typedef ObservableListWrapper<ArrayList<ValueType_> > base;

		ArrayObservableList() : base() { }
		ArrayObservableList(shared_ptr<IEnumerable<typename base::ValueType> > enumerable) : base(enumerable) { }
		ArrayObservableList(shared_ptr<IEnumerator<typename base::ValueType> > enumerator) : base(enumerator) { }
	};

	/** @} */

}


#endif
