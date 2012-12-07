#ifndef STINGRAY_TOOLKIT_SERVICETRAITS_H
#define STINGRAY_TOOLKIT_SERVICETRAITS_H


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

	template < typename ServiceBase_, typename Dependencies_ >
	struct DerivedService : public ServiceBase_
	{
		typedef typename ServiceBase_::ServiceInterfaces									ServiceInterfaces;
		typedef typename TypeListMerge<TypeList_2<typename ServiceBase_::Dependencies,
			typename ToTypeList<Dependencies_>::ValueT> >::ValueT							Dependencies;
	};

	template < typename Service >
	struct GetServiceInterfaces
	{ typedef typename Service::ServiceInterfaces ValueT; };

	template < typename Service >
	struct GetServiceDependencies
	{ typedef typename Service::Dependencies ValueT; };

	/*! \endcond */

}


#endif
