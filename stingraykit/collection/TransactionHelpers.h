#ifndef STINGRAYKIT_COLLECTION_TRANSACTIONHELPERS_H
#define STINGRAYKIT_COLLECTION_TRANSACTIONHELPERS_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/DiffEntry.h>
#include <stingraykit/collection/IEnumerator.h>

namespace stingray
{

	template < typename EnumerationDirection >
	class SortedCollectionTransactionEnumerator : public virtual IEnumerator<typename EnumerationDirection::ItemType>
	{
		STINGRAYKIT_NONCOPYABLE(SortedCollectionTransactionEnumerator);

	public:
		using base = IEnumerator<typename EnumerationDirection::ItemType>;
		using ItemType = typename base::ItemType;

	private:
		using CollectionType = typename EnumerationDirection::CollectionType;
		STINGRAYKIT_DECLARE_PTR(CollectionType);

		using IteratorType = typename EnumerationDirection::IteratorType;
		using LessComparer = typename EnumerationDirection::LessComparer;

		struct EntryType
		{
			STINGRAYKIT_ENUM_VALUES(None, Items, Added, Removed);
			STINGRAYKIT_DECLARE_ENUM_CLASS(EntryType);
		};

	private:
		const CollectionTypePtr			_items;
		const CollectionTypePtr			_added;
		const CollectionTypePtr			_removed;

		EnumerationDirection			_direction;
		EntryType						_entryType;
		IteratorType					_itemsIt;
		IteratorType					_addedIt;
		IteratorType					_removedIt;

	public:
		template < typename... DirectionArgs >
		SortedCollectionTransactionEnumerator(const CollectionTypePtr& items, const CollectionTypePtr& added, const CollectionTypePtr& removed, const DirectionArgs&... args)
			:	_items(STINGRAYKIT_REQUIRE_NOT_NULL(items)),
				_added(STINGRAYKIT_REQUIRE_NOT_NULL(added)),
				_removed(STINGRAYKIT_REQUIRE_NOT_NULL(removed)),
				_direction(args...),
				_itemsIt(_direction.Begin(*_items)),
				_addedIt(_direction.Begin(*_added)),
				_removedIt(_direction.Begin(*_removed))
		{
			UpdateEntryType();
			if (_entryType == EntryType::Removed)
				Next();
		}

		bool Valid() const override
		{ return _entryType == EntryType::Items || _entryType == EntryType::Added; }

		ItemType Get() const override
		{
			switch (_entryType)
			{
			case EntryType::Items:		return _direction.ToItem(*_itemsIt);
			case EntryType::Added:		return _direction.ToItem(*_addedIt);
			default:
				break;
			}

			STINGRAYKIT_THROW("Enumerator is not valid!");
		}

		void Next() override
		{
			do {
				switch (_entryType)
				{
				case EntryType::Items:		++_itemsIt; break;
				case EntryType::Added:		++_addedIt; break;
				case EntryType::Removed:		++_itemsIt; ++_removedIt; break;
				default:
					STINGRAYKIT_THROW("Enumerator is not valid!");
				}

				UpdateEntryType();
			} while (_entryType == EntryType::Removed);
		}

	private:
		void UpdateEntryType()
		{
			if (_itemsIt == _direction.End(*_items))
			{
				_entryType = _addedIt != _direction.End(*_added) ? EntryType::Added : EntryType::None;
				return;
			}

			if (_addedIt != _direction.End(*_added) && LessComparer()(*_addedIt, *_itemsIt))
			{
				_entryType = EntryType::Added;
				return;
			}

			if (_removedIt != _direction.End(*_removed))
			{
				if (LessComparer()(*_itemsIt, *_removedIt))
				{
					_entryType = EntryType::Items;
					return;
				}

				STINGRAYKIT_CHECK(!LessComparer()(*_removedIt, *_itemsIt), LogicException("Broken invariant: 'removed' must be equals to 'item'"));
				_entryType = EntryType::Removed;
				return;
			}

			_entryType = EntryType::Items;
		}
	};


	template < typename EnumerationDirection >
	class SortedCollectionTransactionDiffEnumerator : public virtual IEnumerator<DiffEntry<typename EnumerationDirection::ItemType>>
	{
		STINGRAYKIT_NONCOPYABLE(SortedCollectionTransactionDiffEnumerator);

	public:
		using base = IEnumerator<DiffEntry<typename EnumerationDirection::ItemType>>;
		using ItemType = typename base::ItemType;

	private:
		using CollectionType = typename EnumerationDirection::CollectionType;
		STINGRAYKIT_DECLARE_PTR(CollectionType);

		using IteratorType = typename EnumerationDirection::IteratorType;
		using LessComparer = typename EnumerationDirection::LessComparer;

		struct EntryType
		{
			STINGRAYKIT_ENUM_VALUES(None, Added, Removed);
			STINGRAYKIT_DECLARE_ENUM_CLASS(EntryType);
		};

	private:
		const CollectionTypePtr			_added;
		const CollectionTypePtr			_removed;

		EnumerationDirection			_direction;
		EntryType						_entryType;
		IteratorType					_addedIt;
		IteratorType					_removedIt;

	public:
		template < typename... DirectionArgs >
		SortedCollectionTransactionDiffEnumerator(const CollectionTypePtr& added, const CollectionTypePtr& removed, const DirectionArgs&... args)
			:	_added(STINGRAYKIT_REQUIRE_NOT_NULL(added)),
				_removed(STINGRAYKIT_REQUIRE_NOT_NULL(removed)),
				_direction(args...),
				_addedIt(_direction.Begin(*_added)),
				_removedIt(_direction.Begin(*_removed))
		{ UpdateEntryType(); }

		bool Valid() const override
		{ return _entryType != EntryType::None; }

		ItemType Get() const override
		{
			switch (_entryType)
			{
			case EntryType::Added:		return MakeDiffEntry(CollectionOp::Added, _direction.ToItem(*_addedIt));
			case EntryType::Removed:	return MakeDiffEntry(CollectionOp::Removed, _direction.ToItem(*_removedIt));
			default:
				break;
			}

			STINGRAYKIT_THROW("Enumerator is not valid!");
		}

		void Next() override
		{
			switch (_entryType)
			{
			case EntryType::Added:		++_addedIt; break;
			case EntryType::Removed:	++_removedIt; break;
			default:
				STINGRAYKIT_THROW("Enumerator is not valid!");
			}

			UpdateEntryType();
		}

	private:
		void UpdateEntryType()
		{
			if (_addedIt != _direction.End(*_added))
			{
				if (_removedIt != _direction.End(*_removed))
					_entryType = LessComparer()(*_addedIt, *_removedIt) ? EntryType::Added : EntryType::Removed;
				else
					_entryType = EntryType::Added;
			}
			else if (_removedIt != _direction.End(*_removed))
				_entryType = EntryType::Removed;
			else
				_entryType = EntryType::None;
		}
	};

}

#endif
