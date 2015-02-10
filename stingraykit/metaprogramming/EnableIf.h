#ifndef STINGRAYKIT_METAPROGRAMMING_ENABLEIF_H
#define STINGRAYKIT_METAPROGRAMMING_ENABLEIF_H

namespace stingray
{

	template < bool Cond_, class T > struct EnableIf;
	template < class T> struct EnableIf<true, T> { typedef T	ValueT; };

}

#endif
