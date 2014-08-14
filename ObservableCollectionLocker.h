#ifndef STINGRAY_TOOLKIT_OBSERVABLECOLLECTIONLOCKER_H
#define STINGRAY_TOOLKIT_OBSERVABLECOLLECTIONLOCKER_H


#include <stingray/toolkit/signals.h>


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
		signal_locker _locker;

	public:
		template < typename CollectionType >
		ObservableCollectionLocker(const CollectionType& collection)
			: _locker(collection.OnChanged)
		{ }

		template < typename CollectionType >
		ObservableCollectionLocker(const shared_ptr<CollectionType>& collection)
			: _locker(collection->OnChanged)
		{ }
	};
	TOOLKIT_DECLARE_PTR(ObservableCollectionLocker);

	/** @} */

}


#endif
