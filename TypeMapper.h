#ifndef STINGRAY_TOOLKIT_TYPEMAPPER_H
#define STINGRAY_TOOLKIT_TYPEMAPPER_H


#include <stingray/toolkit/TypeList.h>


namespace stingray
{

	template < typename Derived >
	class TypeMapper
	{
	private:
		template < typename Entry >
		struct MappingChecker
		{
			CompileTimeAssert<!SameType<Entry, TypeListEndNode>::Value> ERROR_no_mapping;
			typedef Entry ValueT;
		};

	public:
		template < typename Src, typename Dst >
		struct Mapping
		{
			typedef Src SrcType;
			typedef Dst DstType;
		};

		template < typename Src >
		struct Map
		{
			template < typename Entry >
			struct SrcFit
			{ static const bool Value = SameType<typename Entry::SrcType, Src>::Value; };

			typedef typename MappingChecker<typename TypeListFindIf<typename Derived::MappingsList, SrcFit>::ValueT>::ValueT::DstType ValueT;
		};

		template < typename Dst >
		struct Unmap
		{
			template < typename Entry >
			struct DstFit
			{ static const bool Value = SameType<typename Entry::DstType, Dst>::Value; };

			typedef typename MappingChecker<typename TypeListFindIf<typename Derived::MappingsList, DstFit>::ValueT>::ValueT::SrcType ValueT;
		};
	};

}


#endif
