#ifndef STINGRAYKIT_BITWISEOPERATIONS_H
#define STINGRAYKIT_BITWISEOPERATIONS_H


#include <stingraykit/Types.h>


#define STINGRAYKIT_MUST_HAVE_FIXED_WIDTH(...) \
	CompileTimeAssert< IsFixedWidthIntType<__VA_ARGS__>::Value > ERROR_Integer_Must_Have_Fixed_Width;


namespace stingray
{


	/**
	 * @ingroup toolkit_bits
	 * @defgroup toolkit_bits_bitwiseops Bitwise operations
	 * @{
	 */

	template < size_t Index, typename Integer >
	bool TestBit(Integer number)
	{
		STINGRAYKIT_MUST_HAVE_FIXED_WIDTH(Integer);
		return number & (1 << Index);
	}


	template< size_t Index, typename Integer >
	Integer SetBit(Integer number)
	{
		STINGRAYKIT_MUST_HAVE_FIXED_WIDTH(Integer);
		return number | (1 << Index);
	}


	template < size_t Index, typename Integer >
	Integer ResetBit(Integer number)
	{
		STINGRAYKIT_MUST_HAVE_FIXED_WIDTH(Integer);
		return number & ~(1 << Index);
	}


	template < size_t Index, typename Integer >
	Integer FlipBit(Integer number)
	{
		STINGRAYKIT_MUST_HAVE_FIXED_WIDTH(Integer);
		return number ^ (1 << Index);
	}

	/** @} */


}


#endif
