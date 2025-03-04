#ifndef STINGRAYKIT_COLLECTION_IOBSERVABLESET_H
#define STINGRAYKIT_COLLECTION_IOBSERVABLESET_H

// Copyright (c) 2011 - 2025, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/CollectionOp.h>
#include <stingraykit/collection/ISet.h>
#include <stingraykit/collection/ObservableCollectionLocker.h>
#include <stingraykit/signal/signal_connector.h>

namespace stingray
{

	/**
	 * @addtogroup toolkit_collections
	 * @{
	 */

	template < typename ValueType_ >
	struct IReadonlyObservableSet
		:	public virtual IReadonlySet<ValueType_>
	{
		using OnChangedSignature = void (CollectionOp, const ValueType_&);

		virtual signal_connector<OnChangedSignature> OnChanged() const = 0;
		virtual const Mutex& GetSyncRoot() const = 0;

		ObservableCollectionLockerPtr Lock() const
		{ return make_shared_ptr<ObservableCollectionLocker>(*this); }
	};


	template < typename T >
	struct IsInheritedIReadonlyObservableSet : public IsInheritedFromTemplate<T, IReadonlyObservableSet>
	{ };


	template < typename ValueType_ >
	struct IObservableSet
		:	public virtual ISet<ValueType_>,
			public virtual IReadonlyObservableSet<ValueType_>
	{ };


	template < typename T >
	struct IsInheritedIObservableSet : public IsInheritedFromTemplate<T, IObservableSet>
	{ };

	/** @} */

}

#endif
