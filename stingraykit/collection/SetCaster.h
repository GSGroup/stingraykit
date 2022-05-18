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

	template < typename SrcType, typename DestType, typename SrcSetType = IReadonlySet<SrcType>>
	class SetCaster : public virtual IReadonlySet<DestType>
	{
		using Self = SetCaster<SrcType, DestType, SrcSetType>;

		STINGRAYKIT_DECLARE_PTR(SrcSetType);

	public:
		using Caster = function<DestType (const SrcType&)>;
		using BackCaster = function<bool (const DestType&, SrcType&)>;

	protected:
		const SrcSetTypePtr									_wrapped;
		const Caster										_caster;
		const BackCaster									_backCaster;

	public:
		SetCaster(const SrcSetTypePtr& wrapped, const Caster& caster, const BackCaster& backCaster)
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

		shared_ptr<IEnumerator<DestType>> Find(const DestType& value) const override
		{
			SrcType value_;
			if (_backCaster(value, value_))
				return WrapEnumerator(_wrapped->Find(value_), _caster);
			return MakeEmptyEnumerator();
		}

		shared_ptr<IEnumerator<DestType>> ReverseFind(const DestType& value) const override
		{
			SrcType value_;
			if (_backCaster(value, value_))
				return WrapEnumerator(_wrapped->ReverseFind(value_), _caster);
			return MakeEmptyEnumerator();
		}
	};


	template < typename SrcType, typename DestType >
	class ObservableSetCaster : public SetCaster<SrcType, DestType, IReadonlyObservableSet<SrcType>>, public virtual IReadonlyObservableSet<DestType>
	{
		using base = SetCaster<SrcType, DestType, IReadonlyObservableSet<SrcType>>;
		using Self = ObservableSetCaster<SrcType, DestType>;

		using SrcSetType = IReadonlyObservableSet<SrcType>;
		STINGRAYKIT_DECLARE_PTR(SrcSetType);

		using ExternalMutexPointer = signal_policies::threading::ExternalMutexPointer;

	public:
		using OnChangedSignature = typename IReadonlyObservableSet<DestType>::OnChangedSignature;

		using Caster = typename base::Caster;
		using BackCaster = typename base::BackCaster;

	private:
		signal<OnChangedSignature, ExternalMutexPointer>	_onChanged;
		const Token											_connection;

	public:
		ObservableSetCaster(const SrcSetTypePtr& wrapped, const Caster& caster, const BackCaster& backCaster)
			:	base(wrapped, caster, backCaster),
				_onChanged(ExternalMutexPointer(shared_ptr<const Mutex>(base::_wrapped, &base::_wrapped->GetSyncRoot())), Bind(&Self::OnChangedPopulator, this, _1)),
				_connection(base::_wrapped->OnChanged().connect(Bind(&Self::ChangedHandler, this, _1, _2)))
		{ }

		signal_connector<OnChangedSignature> OnChanged() const override
		{ return _onChanged.connector(); }

		const Mutex& GetSyncRoot() const override
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
			using SrcType = typename SrcSetType::ItemType;

		private:
			shared_ptr<SrcSetType>	_srcSet;

		public:
			SetCasterProxy(const shared_ptr<SrcSetType>& srcSet) : _srcSet(srcSet)
			{ }

			template < typename DestType >
			operator shared_ptr<IReadonlySet<DestType>> () const
			{ return make_shared_ptr<SetCaster<SrcType, DestType>>(_srcSet, &DefaultCast<DestType>, &DefaultBackCast<DestType>); }

			template < typename DestType >
			operator shared_ptr<IReadonlyObservableSet<DestType>> () const
			{ return make_shared_ptr<ObservableSetCaster<SrcType, DestType>>(_srcSet, &DefaultCast<DestType>, &DefaultBackCast<DestType>); }

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
