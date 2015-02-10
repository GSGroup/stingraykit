#ifndef STINGRAYKIT_METAPROGRAMMING_COMPILETIMEASSERT_H
#define STINGRAYKIT_METAPROGRAMMING_COMPILETIMEASSERT_H

namespace stingray
{

	template < bool Expression > class CompileTimeAssert;
	template < > class CompileTimeAssert<true> { };

}

#endif
