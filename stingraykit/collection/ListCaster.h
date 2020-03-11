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
#include <stingraykit/signal/signals.h>

namespace stingray
{

	/**
	 * @addtogroup toolkit_collections
	 * @{
	 */

	template < typename SrcType, typename DestType, typename SrcListType = IReadonlyList<SrcType> >
	class ListCaster : public virtual IReadonlyList<DestType>
	{
		typedef ListCaster<SrcType, DestType, SrcListType> Self;

		STINGRAYKIT_DECLARE_PTR(SrcListType);

	public:
		typedef function<DestType (const SrcType&)> Caster;
		typedef function<bool (const DestType&, SrcType&)> BackCaster;

	protected:
		const SrcListTypePtr								_wrapped;
		const Caster										_caster;
		const BackCaster									_backCaster;

	public:
		ListCaster(const SrcListTypePtr& wrapped, const Caster& caster, const BackCaster& backCaster)
			: _wrapped(STINGRAYKIT_REQUIRE_NOT_NULL(wrapped)), _caster(caster), _backCaster(backCaster)
		{ }

		virtual shared_ptr<IEnumerator<DestType> > GetEnumerator() const
		{ return WrapEnumerator(_wrapped->GetEnumerator(), _caster); }

		virtual shared_ptr<IEnumerable<DestType> > Reverse() const
		{ return WrapEnumerable(_wrapped->Reverse(), _caster); }

		virtual size_t GetCount() const
		{ return _wrapped->GetCount(); }

		virtual bool IsEmpty() const
		{ return _wrapped->IsEmpty(); }

		virtual bool Contains(const DestType& value) const
		{
			SrcType value_;
			return _backCaster(value, value_) ? _wrapped->Contains(value_) : false;
		}

		virtual optional<size_t> IndexOf(const DestType& value) const
		{
			SrcType value_;
			return _backCaster(value, value_) ? _wrapped->IndexOf(value_) : null;
		}

		virtual DestType Get(size_t index) const
		{ return _caster(_wrapped->Get(index)); }

		virtual bool TryGet(size_t index, DestType& value) const
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
	class ObservableListCaster : public ListCaster<SrcType, DestType, IReadonlyObservableList<SrcType> >, public virtual IReadonlyObservableList<DestType>
	{
		typedef ListCaster<SrcType, DestType, IReadonlyObservableList<SrcType> > base;
		typedef ObservableListCaster<SrcType, DestType> Self;

		typedef IReadonlyObservableList<SrcType> SrcListType;
		STINGRAYKIT_DECLARE_PTR(SrcListType);

		typedef signal_policies::threading::ExternalMutexPointer ExternalMutexPointer;

	public:
		typedef typename IReadonlyObservableList<DestType>::OnChangedSignature OnChangedSignature;

		typedef typename base::Caster Caster;
		typedef typename base::BackCaster BackCaster;

	private:
		signal<OnChangedSignature, ExternalMutexPointer>	_onChanged;
		const Token											_connection;

	public:
		explicit ObservableListCaster(const SrcListTypePtr& wrapped, const Caster& caster, const BackCaster& backCaster)
			:	base(wrapped, caster, backCaster),
				_onChanged(ExternalMutexPointer(shared_ptr<const Mutex>(base::_wrapped, &base::_wrapped->GetSyncRoot())), Bind(&Self::OnChangedPopulator, this, _1)),
				_connection(base::_wrapped->OnChanged().connect(Bind(&Self::ChangedHandler, this, _1, _2, _3)))
		{ }

		virtual signal_connector<OnChangedSignature> OnChanged() const
		{ return _onChanged.connector(); }

		virtual const Mutex& GetSyncRoot() const
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


	namespace Detail
	{

		template < typename SrcListType >
		class ListCasterProxy
		{
			typedef typename SrcListType::ItemType SrcType;

		private:
			shared_ptr<SrcListType>	_srcList;

		public:
			ListCasterProxy(const shared_ptr<SrcListType>& srcList) : _srcList(srcList)
			{ }

			template < typename DestType >
			operator shared_ptr<IReadonlyList<DestType> > () const
			{ return make_shared_ptr<ListCaster<SrcType, DestType> >(_srcList, &DefaultCast<DestType>, &DefaultBackCast<DestType>); }

			template < typename DestType >
			operator shared_ptr<IReadonlyObservableList<DestType> > () const
			{ return make_shared_ptr<ObservableListCaster<SrcType, DestType> >(_srcList, &DefaultCast<DestType>, &DefaultBackCast<DestType>); }

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
