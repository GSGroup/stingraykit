#ifndef STINGRAYKIT_METAPROGRAMMING_INTTOTYPE_H
#define STINGRAYKIT_METAPROGRAMMING_INTTOTYPE_H

namespace stingray
{

	template < int N >
	struct IntToType
	{ static const int Value = N; };

}

#endif
