#ifndef STINGRAYKIT_METAPROGRAMMING_INTEGRALCONSTANT_H
#define STINGRAYKIT_METAPROGRAMMING_INTEGRALCONSTANT_H

namespace stingray
{

	template<typename T_, T_ Value_>
	struct integral_constant
	{
		typedef T_ ValueT;
		static const ValueT Value = Value_;
	};

	template<typename Val1, typename Val2>
	struct integral_constant_less
	{ static const bool Value = Val1::Value < Val2::Value; };

}

#endif
