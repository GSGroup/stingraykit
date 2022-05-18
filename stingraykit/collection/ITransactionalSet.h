#ifndef STINGRAYKIT_COLLECTION_ITRANSACTIONALSET_H
#define STINGRAYKIT_COLLECTION_ITRANSACTIONALSET_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/DiffEntry.h>
#include <stingraykit/collection/ISet.h>
#include <stingraykit/signal/signal_connector.h>

namespace stingray
{

	/**
	 * @addtogroup toolkit_collections
	 * @{
	 */

	template < typename T >
	struct ISetTransaction : public virtual ISet<T>
	{
		using DiffEntryType = DiffEntry<T>;
		using DiffType = IEnumerable<DiffEntryType>;
		STINGRAYKIT_DECLARE_PTR(DiffType);

		virtual void Apply(const DiffEntryType& entry) = 0;

		virtual DiffTypePtr Diff() const = 0;
		virtual bool IsDirty() const = 0;

		virtual void Commit() = 0;
		virtual void Revert() = 0;
	};


	template < typename T >
	struct IReadonlyTransactionalSet : public virtual IReadonlySet<T>
	{
		using DiffEntryType = DiffEntry<T>;
		using DiffType = IEnumerable<DiffEntryType>;
		STINGRAYKIT_DECLARE_PTR(DiffType);

		virtual signal_connector<void (const DiffTypePtr&)> OnChanged() const = 0;
		virtual const Mutex& GetSyncRoot() const = 0;
	};


	template < typename T >
	struct ITransactionalSet : public virtual IReadonlyTransactionalSet<T>
	{
		using TransactionType = ISetTransaction<T>;
		STINGRAYKIT_DECLARE_PTR(TransactionType);

		virtual TransactionTypePtr StartTransaction(const ICancellationToken& token = DummyCancellationToken()) = 0;
	};

	/** @} */

}

#endif
