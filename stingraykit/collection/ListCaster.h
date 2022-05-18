#ifndef STINGRAYKIT_COLLECTION_LISTCASTER_H
#define STINGRAYKIT_COLLECTION_LISTCASTER_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/EnumerableHelpers.h>
#include <stingraykit/collection/ForEach.h>
#include <stingraykit/collection/IObservableList.h>
#include <stingraykit/collection/ITransactionalList.h>
#include <stingraykit/signal/signals.h>

namespace stingray
{

	/**
	 * @addtogroup toolkit_collections
	 * @{
	 */

	template < typename SrcType, typename DestType, typename SrcListType = IReadonlyList<SrcType>>
	class ListCaster : public virtual IReadonlyList<DestType>
	{
		using Self = ListCaster<SrcType, DestType, SrcListType>;

		STINGRAYKIT_DECLARE_PTR(SrcListType);

	public:
		using Caster = function<DestType (const SrcType&)>;
		using BackCaster = function<bool (const DestType&, SrcType&)>;

	protected:
		const SrcListTypePtr								_wrapped;
		const Caster										_caster;
		const BackCaster									_backCaster;

	public:
		ListCaster(const SrcListTypePtr& wrapped, const Caster& caster, const BackCaster& backCaster)
			: _wrapped(STINGRAYKIT_REQUIRE_NOT_NULL(wrapped)), _caster(caster), _backCaster(backCaster)
		{ }

		shared_ptr<IEnumerator<DestType>> GetEnumerator() const override
		{ return WrapEnumerator(_wrapped->GetEnumerator(), _caster); }

		shared_ptr<IEnumerable<DestType>> Reverse() const override
		{ return WrapEnumerable(_wrapped->Reverse(), _caster); }

		size_t GetCount() const override
		{ return _wrapped->GetCount(); }

		bool IsEmpty() const override
		{ return _wrapped->IsEmpty(); }

		bool Contains(const DestType& value) const override
		{
			SrcType value_;
			return _backCaster(value, value_) ? _wrapped->Contains(value_) : false;
		}

		optional<size_t> IndexOf(const DestType& value) const override
		{
			SrcType value_;
			return _backCaster(value, value_) ? _wrapped->IndexOf(value_) : null;
		}

		DestType Get(size_t index) const override
		{ return _caster(_wrapped->Get(index)); }

		bool TryGet(size_t index, DestType& value) const override
		{
			SrcType value_;
			if (_wrapped->TryGet(index, value_))
			{
				value = _caster(value_);
				return true;
			}
			return false;
		}
	};


	template < typename SrcType, typename DestType >
	class ObservableListCaster : public ListCaster<SrcType, DestType, IReadonlyObservableList<SrcType>>, public virtual IReadonlyObservableList<DestType>
	{
		using base = ListCaster<SrcType, DestType, IReadonlyObservableList<SrcType>>;
		using Self = ObservableListCaster<SrcType, DestType>;

		using SrcListType = IReadonlyObservableList<SrcType>;
		STINGRAYKIT_DECLARE_PTR(SrcListType);

		using DestListType = IReadonlyObservableList<DestType>;

		using ExternalMutexPointer = signal_policies::threading::ExternalMutexPointer;

	public:
		using OnChangedSignature = typename DestListType::OnChangedSignature;

		using Caster = typename base::Caster;
		using BackCaster = typename base::BackCaster;

	private:
		signal<OnChangedSignature, ExternalMutexPointer>	_onChanged;
		const Token											_connection;

	public:
		ObservableListCaster(const SrcListTypePtr& wrapped, const Caster& caster, const BackCaster& backCaster)
			:	base(wrapped, caster, backCaster),
				_onChanged(ExternalMutexPointer(shared_ptr<const Mutex>(base::_wrapped, &base::_wrapped->GetSyncRoot())), Bind(&Self::OnChangedPopulator, this, _1)),
				_connection(base::_wrapped->OnChanged().connect(Bind(&Self::ChangedHandler, this, _1, _2, _3)))
		{ }

		signal_connector<OnChangedSignature> OnChanged() const override
		{ return _onChanged.connector(); }

		const Mutex& GetSyncRoot() const override
		{ return base::_wrapped->GetSyncRoot(); }

	private:
		void OnChangedPopulator(const function<OnChangedSignature>& slot) const
		{
			size_t index = 0;
			FOR_EACH(const DestType value IN base::GetEnumerator())
				slot(CollectionOp::Added, index++, value);
		}

		void ChangedHandler(CollectionOp op, size_t index, const SrcType& value)
		{ _onChanged(op, index, base::_caster(value)); }
	};


	template < typename SrcType, typename DestType >
	class TransactionalListCaster : public ListCaster<SrcType, DestType, IReadonlyTransactionalList<SrcType>>, public virtual IReadonlyTransactionalList<DestType>
	{
		using base = ListCaster<SrcType, DestType, IReadonlyTransactionalList<SrcType>>;
		using Self = TransactionalListCaster<SrcType, DestType>;

		using SrcListType = IReadonlyTransactionalList<SrcType>;
		STINGRAYKIT_DECLARE_PTR(SrcListType);

		using DestListType = IReadonlyTransactionalList<DestType>;

		using ExternalMutexPointer = signal_policies::threading::ExternalMutexPointer;

	public:
		using OnChangedSignature = typename DestListType::OnChangedSignature;

		using Caster = typename base::Caster;
		using BackCaster = typename base::BackCaster;

	private:
		signal<OnChangedSignature, ExternalMutexPointer>	_onChanged;
		const Token											_connection;

	public:
		TransactionalListCaster(const SrcListTypePtr& wrapped, const Caster& caster, const BackCaster& backCaster)
			:	base(wrapped, caster, backCaster),
				_onChanged(ExternalMutexPointer(shared_ptr<const Mutex>(base::_wrapped, &base::_wrapped->GetSyncRoot())), Bind(&Self::OnChangedPopulator, this, _1)),
				_connection(base::_wrapped->OnChanged().connect(Bind(&Self::ConvertDiffAndInvoke, this, _onChanged.invoker(), _1)))
		{ }

		signal_connector<OnChangedSignature> OnChanged() const override
		{ return _onChanged.connector(); }

		const Mutex& GetSyncRoot() const override
		{ return base::_wrapped->GetSyncRoot(); }

	private:
		void OnChangedPopulator(const function<OnChangedSignature>& slot) const
		{ base::_wrapped->OnChanged().SendCurrentState(Bind(&Self::ConvertDiffAndInvoke, this, slot, _1)); }

		void ConvertDiffAndInvoke(const function<OnChangedSignature>& slot, const typename SrcListType::DiffTypePtr& diff) const
		{ slot(Enumerable::Transform(diff, Bind(&Self::ConvertDiffEntry, base::_caster, _1))); }

		static typename DestListType::DiffEntryType ConvertDiffEntry(const Caster& caster, const typename SrcListType::DiffEntryType& entry)
		{ return MakeDiffEntry(entry.Op, typename DestListType::PairType(entry.Item.Key, caster(entry.Item.Value))); }
	};


	namespace Detail
	{

		template < typename SrcListType >
		class ListCasterProxy
		{
			using SrcType = typename SrcListType::ItemType;

		private:
			shared_ptr<SrcListType>	_srcList;

		public:
			ListCasterProxy(const shared_ptr<SrcListType>& srcList) : _srcList(srcList)
			{ }

			template < typename DestType >
			operator shared_ptr<IReadonlyList<DestType>> () const
			{ return make_shared_ptr<ListCaster<SrcType, DestType>>(_srcList, &DefaultCast<DestType>, &DefaultBackCast<DestType>); }

			template < typename DestType >
			operator shared_ptr<IReadonlyObservableList<DestType>> () const
			{ return make_shared_ptr<ObservableListCaster<SrcType, DestType>>(_srcList, &DefaultCast<DestType>, &DefaultBackCast<DestType>); }

			template < typename DestType >
			operator shared_ptr<IReadonlyTransactionalList<DestType>> () const
			{ return make_shared_ptr<TransactionalListCaster<SrcType, DestType>>(_srcList, &DefaultCast<DestType>, &DefaultBackCast<DestType>); }

		private:
			template < typename DestType >
			static typename EnableIf<IsConvertible<SrcType, DestType>::Value, DestType>::ValueT DefaultCast(const SrcType& value)
			{ return DestType(value); }

			template < typename DestType >
			static typename EnableIf<IsSharedPtr<SrcType>::Value && IsSharedPtr<DestType>::Value, bool>::ValueT DefaultBackCast(const DestType& value, SrcType& result)
			{
				result = dynamic_caster(value);
				return result.is_initialized();
			}
		};

	}


	template < typename T >
	typename Detail::ListCasterProxy<T> GetListCaster(const shared_ptr<T>& list)
	{ return Detail::ListCasterProxy<T>(list); }

	/** @} */

}

#endif
