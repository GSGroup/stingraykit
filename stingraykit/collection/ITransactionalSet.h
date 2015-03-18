#ifndef STINGRAYKIT_COLLECTION_ITRANSACTIONALSET_H
#define STINGRAYKIT_COLLECTION_ITRANSACTIONALSET_H

// Copyright (c) 2011 - 2015, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/DiffEntry.h>
#include <stingraykit/collection/ISet.h>
#include <stingraykit/collection/ObservableCollectionLocker.h>
#include <stingraykit/signal/signals.h>
#include <stingraykit/toolkit.h>

namespace stingray
{

	/**
	 * @addtogroup toolkit_collections
	 * @{
	 */

	template < typename T >
	struct ISetTransaction : public virtual ISet<T>
	{
		typedef ISet<T>						base;
		typedef DiffEntry<T>				DiffEntryType;
		typedef IEnumerable<DiffEntryType>	DiffType;
		STINGRAYKIT_DECLARE_PTR(DiffType);

		virtual DiffTypePtr Diff() const = 0;

		void Apply(const DiffEntryType& entry)
		{
			switch (entry.Op)
			{
			case CollectionOp::Added:
				this->Add(entry.Item);
				break;
			case CollectionOp::Removed:
				this->Remove(entry.Item);
				break;
			case CollectionOp::Updated:
			default:
				STINGRAYKIT_THROW(StringBuilder() % "Not supported CollectionOp: " % entry.Op);
			}
		}

		virtual void Commit() = 0;
		virtual void Revert() = 0;
	};


	template < typename T >
	struct ITransactionalSet : public virtual ISet<T>
	{
		typedef IReadonlySet<T>				base;

		typedef DiffEntry<T>				DiffEntryType;
		typedef IEnumerable<DiffEntryType>	DiffType;
		STINGRAYKIT_DECLARE_PTR(DiffType);

		typedef ISetTransaction<T>			Transaction;
		STINGRAYKIT_DECLARE_PTR(Transaction);

		virtual const Mutex& GetSync() const = 0;
		virtual signal_connector<void(const DiffTypePtr&)> OnChanged() const = 0;

		virtual TransactionPtr StartTransaction() = 0;
	};

	/** @} */

}


#endif

