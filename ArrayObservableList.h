#ifndef STINGRAY_TOOLKIT_ARRAYOBSERVABLELIST_H
#define STINGRAY_TOOLKIT_ARRAYOBSERVABLELIST_H


#include <stingray/toolkit/ArrayList.h>
#include <stingray/toolkit/IObservableList.h>


namespace stingray
{

	template < typename ValueType_ >
	class ArrayObservableList : public ObservableListWrapper<ArrayList<ValueType_> >
	{ };

}


#endif
