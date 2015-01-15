#ifndef STINGRAYKIT_COLLECTION_ARRAYOBSERVABLELIST_H
#define STINGRAYKIT_COLLECTION_ARRAYOBSERVABLELIST_H


#include <stingraykit/collection/ArrayList.h>
#include <stingraykit/collection/IObservableList.h>


namespace stingray
{

	/**
	 * @addtogroup toolkit_collections
	 * @{
	 */

	template < typename ValueType_ >
	class ArrayObservableList : public ObservableListWrapper<ArrayList<ValueType_> >
	{ };

	/** @} */

}


#endif
