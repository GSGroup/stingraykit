#ifndef STINGRAYKIT_METAPROGRAMMING_TYPEMAPPER_H
#define STINGRAYKIT_METAPROGRAMMING_TYPEMAPPER_H

// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <stingraykit/metaprogramming/TypeList.h>
#include <stingraykit/metaprogramming/TypeRelationships.h>


namespace stingray
{

	template < typename Derived >
	class TypeMapper
	{
	private:
		template < typename Entry >
		struct MappingChecker
		{
			CompileTimeAssert<!IsSame<Entry, TypeListEndNode>::Value> ERROR_no_mapping;
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
			struct SrcFit : IsSame<typename Entry::SrcType, Src> { };

			typedef typename MappingChecker<typename TypeListFindIf<typename Derived::MappingsList, SrcFit>::ValueT>::ValueT::DstType ValueT;
		};

		template < typename Dst >
		struct Unmap
		{
			template < typename Entry >
			struct DstFit : IsSame<typename Entry::DstType, Dst> { };

			typedef typename MappingChecker<typename TypeListFindIf<typename Derived::MappingsList, DstFit>::ValueT>::ValueT::SrcType ValueT;
		};
	};

}


#endif
