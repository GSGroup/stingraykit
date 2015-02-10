#ifndef STINGRAYKIT_METAPROGRAMMING_IF_H
#define STINGRAYKIT_METAPROGRAMMING_IF_H

namespace stingray
{

	template < bool Expression, typename IfType, typename ElseType >
	struct If;

	template < typename IfType, typename ElseType >
	struct If<true, IfType, ElseType> { typedef IfType ValueT; };

	template < typename IfType, typename ElseType >
	struct If<false, IfType, ElseType> { typedef ElseType ValueT; };

}

#endif
