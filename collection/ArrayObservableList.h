#ifndef STINGRAY_TOOLKIT_COLLECTIONS_ARRAYOBSERVABLELIST_H
#define STINGRAY_TOOLKIT_COLLECTIONS_ARRAYOBSERVABLELIST_H


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
