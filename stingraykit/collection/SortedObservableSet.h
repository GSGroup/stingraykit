#ifndef STINGRAYKIT_COLLECTION_SORTEDOBSERVABLESET_H
#define STINGRAYKIT_COLLECTION_SORTEDOBSERVABLESET_H


#include <stingraykit/collection/IObservableSet.h>
#include <stingraykit/collection/SortedSet.h>

namespace stingray
{

	/**
	 * @addtogroup toolkit_collections
	 * @{
	 */

	template < typename ValueType_ , typename CompareType_ = std::less<ValueType_> >
	class SortedObservableSet : public ObservableSetWrapper<SortedSet<ValueType_, CompareType_> >
	{
		typedef ObservableSetWrapper<SortedSet<ValueType_, CompareType_> > base;
		typedef signal_policies::threading::ExternalMutexPointer ExternalMutexPointer;

	private:
		signal<void(CollectionOp, const ValueType_&), ExternalMutexPointer>	_onChanged;

	public:
		SortedObservableSet() : _onChanged(ExternalMutexPointer(base::GetMutexPointer()), bind(&SortedObservableSet::OnChangedPopulator, this, _1))
		{ }

		virtual signal_connector<void(CollectionOp, const ValueType_&)>	OnChanged() const
		{ return _onChanged.connector(); }

		virtual const Mutex& GetSyncRoot() const
		{ return *base::GetMutexPointer(); }

	protected:
		virtual void InvokeOnChanged(CollectionOp op, const ValueType_& value)
		{ _onChanged(op, value); }

		void OnChangedPopulator(const function<void(CollectionOp, const ValueType_&)>& slot)
		{
			FOR_EACH(ValueType_ v IN this->GetEnumerator())
				slot(CollectionOp::Added, v);
		}
	};

	/** @} */

}


#endif
