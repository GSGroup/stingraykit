#ifndef STINGRAYKIT_METAPROGRAMMING_TOVOID_H
#define STINGRAYKIT_METAPROGRAMMING_TOVOID_H

namespace stingray
{

	template < typename > struct ToVoid { typedef void ValueT; };

}

#endif
