#ifndef STINGRAYKIT_METAPROGRAMMING_MATH_H
#define STINGRAYKIT_METAPROGRAMMING_MATH_H

namespace stingray
{

	template<typename ResultT, ResultT Base, unsigned int Exponent>
	struct CompileTimeExponent { static const ResultT Value = Base * CompileTimeExponent<ResultT, Base, Exponent - 1>::Value; };

	template<typename ResultT, ResultT Base>
	struct CompileTimeExponent<ResultT, Base, 0> { static const ResultT Value = 1; };


	template <unsigned int Val, unsigned int Boundary>
	struct CompileTimeAlignDown
	{
		CompileTimeAssert<Boundary != 0> ERROR__aligning_to_zero_boundary;
		static const unsigned int Value = Boundary * (Val / Boundary);
	};

	template <unsigned int Val, unsigned int Boundary>
	struct CompileTimeAlignUp
	{
		CompileTimeAssert<Boundary != 0> ERROR__aligning_to_zero_boundary;
		static const unsigned int Value = Boundary * ((Val + Boundary - 1) / Boundary);
	};

}

#endif
