#ifndef STINGRAYKIT_COLLECTION_ARRAYOBSERVABLELIST_H
#define STINGRAYKIT_COLLECTION_ARRAYOBSERVABLELIST_H


#include <stingray/toolkit/collection/ArrayList.h>
#include <stingray/toolkit/collection/IObservableList.h>


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
