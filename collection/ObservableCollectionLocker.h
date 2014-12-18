#ifndef STINGRAY_TOOLKIT_COLLECTION_OBSERVABLECOLLECTIONLOCKER_H
#define STINGRAY_TOOLKIT_COLLECTION_OBSERVABLECOLLECTIONLOCKER_H


#include <stingray/toolkit/signal/signals.h>


namespace stingray
{

	/**
	 * @addtogroup toolkit_collections
	 * @{
	 */

	class ObservableCollectionLocker
	{
		TOOLKIT_NONCOPYABLE(ObservableCollectionLocker);

	private:
		MutexLock		_lock;

	public:
		template < typename CollectionType >
		ObservableCollectionLocker(const CollectionType& collection)
			: _lock(collection.GetSyncRoot())
		{ }

		template < typename CollectionType >
		ObservableCollectionLocker(const shared_ptr<CollectionType>& collection)
			: _lock(collection->GetSyncRoot())
		{ }
	};
	TOOLKIT_DECLARE_PTR(ObservableCollectionLocker);

	/** @} */

}


#endif
