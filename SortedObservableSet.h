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
	{ };

	/** @} */

}


#endif
