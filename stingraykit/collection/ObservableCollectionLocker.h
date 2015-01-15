#ifndef STINGRAYKIT_COLLECTION_OBSERVABLECOLLECTIONLOCKER_H
#define STINGRAYKIT_COLLECTION_OBSERVABLECOLLECTIONLOCKER_H


#include <stingraykit/signal/signals.h>


namespace stingray
{

	/**
	 * @addtogroup toolkit_collections
	 * @{
	 */

	class ObservableCollectionLocker
	{
		STINGRAYKIT_NONCOPYABLE(ObservableCollectionLocker);

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
	STINGRAYKIT_DECLARE_PTR(ObservableCollectionLocker);

	/** @} */

}


#endif
