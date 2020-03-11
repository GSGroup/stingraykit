#ifndef STINGRAYKIT_COLLECTION_SETCASTER_H
#define STINGRAYKIT_COLLECTION_SETCASTER_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/EnumerableHelpers.h>
#include <stingraykit/collection/ForEach.h>
#include <stingraykit/collection/IObservableSet.h>
#include <stingraykit/signal/signals.h>

namespace stingray
{

	/**
	 * @addtogroup toolkit_collections
	 * @{
	 */

	template < typename SrcType, typename DestType, typename SrcSetType = IReadonlySet<SrcType> >
	class SetCaster : public virtual IReadonlySet<DestType>
	{
		typedef SetCaster<SrcType, DestType, SrcSetType> Self;

		STINGRAYKIT_DECLARE_PTR(SrcSetType);

	public:
		typedef function<DestType (const SrcType&)> Caster;
		typedef function<bool (const DestType&, SrcType&)> BackCaster;

	protected:
		const SrcSetTypePtr									_wrapped;
		const Caster										_caster;
		const BackCaster									_backCaster;

	public:
		SetCaster(const SrcSetTypePtr& wrapped, const Caster& caster, const BackCaster& backCaster)
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

		virtual shared_ptr<IEnumerator<DestType> > Find(const DestType& value) const
		{
			SrcType value_;
			if (_backCaster(value, value_))
				return WrapEnumerator(_wrapped->Find(value_), _caster);
			return MakeEmptyEnumerator();
		}

		virtual shared_ptr<IEnumerator<DestType> > ReverseFind(const DestType& value) const
		{
			SrcType value_;
			if (_backCaster(value, value_))
				return WrapEnumerator(_wrapped->ReverseFind(value_), _caster);
			return MakeEmptyEnumerator();
		}
	};


	template < typename SrcType, typename DestType >
	class ObservableSetCaster : public SetCaster<SrcType, DestType, IReadonlyObservableSet<SrcType> >, public virtual IReadonlyObservableSet<DestType>
	{
		typedef SetCaster<SrcType, DestType, IReadonlyObservableSet<SrcType> > base;
		typedef ObservableSetCaster<SrcType, DestType> Self;

		typedef IReadonlyObservableSet<SrcType> SrcSetType;
		STINGRAYKIT_DECLARE_PTR(SrcSetType);

		typedef signal_policies::threading::ExternalMutexPointer ExternalMutexPointer;

	public:
		typedef typename IReadonlyObservableSet<DestType>::OnChangedSignature OnChangedSignature;

		typedef typename base::Caster Caster;
		typedef typename base::BackCaster BackCaster;

	private:
		signal<OnChangedSignature, ExternalMutexPointer>	_onChanged;
		const Token											_connection;

	public:
		explicit ObservableSetCaster(const SrcSetTypePtr& wrapped, const Caster& caster, const BackCaster& backCaster)
			:	base(wrapped, caster, backCaster),
				_onChanged(ExternalMutexPointer(shared_ptr<const Mutex>(base::_wrapped, &base::_wrapped->GetSyncRoot())), Bind(&Self::OnChangedPopulator, this, _1)),
				_connection(base::_wrapped->OnChanged().connect(Bind(&Self::ChangedHandler, this, _1, _2)))
		{ }

		virtual signal_connector<OnChangedSignature> OnChanged() const
		{ return _onChanged.connector(); }

		virtual const Mutex& GetSyncRoot() const
		{ return base::_wrapped->GetSyncRoot(); }

	private:
		void OnChangedPopulator(const function<OnChangedSignature>& slot) const
		{
			FOR_EACH(const DestType value IN base::GetEnumerator())
				slot(CollectionOp::Added, value);
		}

		void ChangedHandler(CollectionOp op, const SrcType& value)
		{ _onChanged(op, base::_caster(value)); }
	};


	namespace Detail
	{

		template < typename SrcSetType >
		class SetCasterProxy
		{
			typedef typename SrcSetType::ItemType SrcType;

		private:
			shared_ptr<SrcSetType>	_srcSet;

		public:
			SetCasterProxy(const shared_ptr<SrcSetType>& srcSet) : _srcSet(srcSet)
			{ }

			template < typename DestType >
			operator shared_ptr<IReadonlySet<DestType> > () const
			{ return make_shared_ptr<SetCaster<SrcType, DestType> >(_srcSet, &DefaultCast<DestType>, &DefaultBackCast<DestType>); }

			template < typename DestType >
			operator shared_ptr<IReadonlyObservableSet<DestType> > () const
			{ return make_shared_ptr<ObservableSetCaster<SrcType, DestType> >(_srcSet, &DefaultCast<DestType>, &DefaultBackCast<DestType>); }

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
	typename Detail::SetCasterProxy<T> GetSetCaster(const shared_ptr<T>& set)
	{ return Detail::SetCasterProxy<T>(set); }

	/** @} */

}

#endif
