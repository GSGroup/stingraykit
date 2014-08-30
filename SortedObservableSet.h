#ifndef STINGRAY_TOOLKIT_SORTEDOBSERVABLESET_H
#define STINGRAY_TOOLKIT_SORTEDOBSERVABLESET_H


#include <stingray/toolkit/IObservableSet.h>
#include <stingray/toolkit/SortedSet.h>

namespace stingray
{

	/**
	 * @addtogroup toolkit_collections
	 * @{
	 */

	template < typename ValueType_ , typename CompareType_ = std::less<ValueType_> >
	class SortedObservableSet : public ObservableSetWrapper<SortedSet<ValueType_, CompareType_> >
	{
		typedef signal_policies::threading::ExternalMutexPointer ExternalMutexPointer;

	private:
		shared_ptr<Mutex>													_mutex;
		signal<void(CollectionOp, const ValueType_&), ExternalMutexPointer>	_onChanged;

	public:
		SortedObservableSet() : _mutex(new Mutex()), _onChanged(ExternalMutexPointer(_mutex), bind(&SortedObservableSet::OnChangedPopulator, this, _1))
		{ }

		virtual signal_connector<void(CollectionOp, const ValueType_&)>	OnChanged() const
		{ return _onChanged.connector(); }

		virtual const Mutex& GetSyncRoot() const
		{ return *_mutex; }

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
