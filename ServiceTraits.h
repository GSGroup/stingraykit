#ifndef __GS_DVRLIB_TOOLKIT_SERVICETRAITS_H__
#define __GS_DVRLIB_TOOLKIT_SERVICETRAITS_H__


#include <dvrlib/toolkit/TypeList.h>


namespace dvrlib
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
