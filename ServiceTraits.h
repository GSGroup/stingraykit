#ifndef STINGRAY_TOOLKIT_SERVICETRAITS_H
#define STINGRAY_TOOLKIT_SERVICETRAITS_H


#include <stingray/toolkit/TypeList.h>


namespace stingray
{

	/**
	 * @addtogroup core_services
	 * @{
	 */

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

		DerivedService() { }

		template < typename T > DerivedService(const T& p) : ServiceBase_(p) { }
		template < typename T1, typename T2 > DerivedService(const T1& p1, const T2& p2) : ServiceBase_(p1, p2) { }
	};

	template < typename Service >
	struct GetServiceInterfaces
	{ typedef typename Service::ServiceInterfaces ValueT; };

	template < typename Service >
	struct GetServiceDependencies
	{ typedef typename Service::Dependencies ValueT; };

	/** @} */

}


#endif
