#ifndef STINGRAY_TOOLKIT_COLLECTION_MAPOBSERVABLEDICTIONARY_H
#define STINGRAY_TOOLKIT_COLLECTION_MAPOBSERVABLEDICTIONARY_H


#include <stingray/toolkit/collection/IObservableDictionary.h>
#include <stingray/toolkit/collection/MapDictionary.h>


namespace stingray
{

	/**
	 * @addtogroup toolkit_collections
	 * @{
	 */

	template < typename KeyType_, typename ValueType_, typename CompareType_ = std::less<KeyType_> >
	class MapObservableDictionary : public ObservableDictionaryWrapper<MapDictionary<KeyType_, ValueType_, CompareType_> >
	{ };

	/** @} */

}


#endif
