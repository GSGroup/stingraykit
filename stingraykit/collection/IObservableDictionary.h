#ifndef STINGRAYKIT_COLLECTION_IOBSERVABLEDICTIONARY_H
#define STINGRAYKIT_COLLECTION_IOBSERVABLEDICTIONARY_H

// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/CollectionOp.h>
#include <stingraykit/collection/IDictionary.h>
#include <stingraykit/collection/ObservableCollectionLocker.h>
#include <stingraykit/signal/signal_connector.h>

namespace stingray
{

	/**
	 * @addtogroup toolkit_collections
	 * @{
	 */

	template < typename KeyType_, typename ValueType_ >
	struct IReadonlyObservableDictionary
		:	public virtual IReadonlyDictionary<KeyType_, ValueType_>
	{
		using KeyPassingType = typename GetParamPassingType<KeyType_>::ValueT;
		using ValuePassingType = typename GetParamPassingType<ValueType_>::ValueT;
		using OnChangedSignature = void (CollectionOp, KeyPassingType, ValuePassingType);

	public:
		virtual signal_connector<OnChangedSignature> OnChanged() const = 0;
		virtual const Mutex& GetSyncRoot() const = 0;

		ObservableCollectionLockerPtr Lock() const
		{ return make_shared_ptr<ObservableCollectionLocker>(*this); }
	};


	template < typename T >
	struct IsInheritedIReadonlyObservableDictionary : public IsInheritedFromTemplate<T, IReadonlyObservableDictionary>
	{ };


	template < typename KeyType_, typename ValueType_ >
	struct IObservableDictionary
		:	public virtual IDictionary<KeyType_, ValueType_>,
			public virtual IReadonlyObservableDictionary<KeyType_, ValueType_>
	{ };


	template < typename T >
	struct IsInheritedIObservableDictionary : public IsInheritedFromTemplate<T, IObservableDictionary>
	{ };

	/** @} */

}

#endif
