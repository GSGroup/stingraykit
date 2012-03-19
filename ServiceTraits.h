#ifndef __GS_DVRLIB_TOOLKIT_SERVICETRAITS_H__
#define __GS_DVRLIB_TOOLKIT_SERVICETRAITS_H__


#include <stingray/toolkit/TypeList.h>


namespace stingray
{

	/*! \cond GS_INTERNAL */

	template < typename ServiceInterfaces_, typename Dependencies_ >
	struct ServiceTraits
	{
		typedef typename ToTypeList<ServiceInterfaces_>::ValueT	ServiceInterfaces;
		typedef typename ToTypeList<Dependencies_>::ValueT		Dependencies;
	};

	/*! \endcond */

}


#endif
