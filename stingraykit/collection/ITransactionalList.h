#ifndef STINGRAYKIT_COLLECTION_ITRANSACTIONALLIST_H
#define STINGRAYKIT_COLLECTION_ITRANSACTIONALLIST_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/DiffEntry.h>
#include <stingraykit/collection/IList.h>
#include <stingraykit/collection/KeyValuePair.h>
#include <stingraykit/collection/ObservableCollectionLocker.h>
#include <stingraykit/signal/signal_connector.h>

namespace stingray
{

	/**
	 * @addtogroup toolkit_collections
	 * @{
	 */

	template < typename ValueType_ >
	struct IListTransaction : public virtual IList<ValueType_>
	{
		using base = IList<ValueType_>;

		using ValueType = typename base::ValueType;
		using PairType = KeyValuePair<size_t, ValueType>;

		using DiffEntryType = DiffEntry<PairType>;
		using DiffType = IEnumerable<DiffEntryType>;
		STINGRAYKIT_DECLARE_PTR(DiffType);

		virtual void Apply(const DiffEntryType& entry) = 0;

		virtual DiffTypePtr Diff() const = 0;
		virtual bool IsDirty() const = 0;

		virtual void Commit() = 0;
		virtual void Revert() = 0;
	};


	template < typename ValueType_ >
	struct IReadonlyTransactionalList : public virtual IReadonlyList<ValueType_>
	{
		using base = IReadonlyList<ValueType_>;

		using ValueType = typename base::ValueType;
		using PairType = KeyValuePair<size_t, ValueType>;

		using DiffEntryType = DiffEntry<PairType>;
		using DiffType = IEnumerable<DiffEntryType>;
		STINGRAYKIT_DECLARE_PTR(DiffType);

		using OnChangedSignature = void (const DiffTypePtr&);

		virtual signal_connector<OnChangedSignature> OnChanged() const = 0;
		virtual const Mutex& GetSyncRoot() const = 0;

		ObservableCollectionLockerPtr Lock() const
		{ return make_shared_ptr<ObservableCollectionLocker>(*this); }
	};


	template < typename T >
	struct IsInheritedIReadonlyTransactionalList : public IsInherited1ParamTemplate<T, IReadonlyTransactionalList>
	{ };


	template < typename ValueType_ >
	struct ITransactionalList : public virtual IReadonlyTransactionalList<ValueType_>
	{
		using TransactionType = IListTransaction<ValueType_>;
		STINGRAYKIT_DECLARE_PTR(TransactionType);

		virtual TransactionTypePtr StartTransaction(const ICancellationToken& token = DummyCancellationToken()) = 0;
	};


	template < typename T >
	struct IsInheritedITransactionalList : public IsInherited1ParamTemplate<T, ITransactionalList>
	{ };

	/** @} */

}

#endif
