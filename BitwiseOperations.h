#ifndef __GS_DVRLIB_TOOLKIT_BITWISEOPERATIONS_H__
#define __GS_DVRLIB_TOOLKIT_BITWISEOPERATIONS_H__


#include <dvrlib/toolkit/Types.h>


#define TOOLKIT_MUST_HAVE_FIXED_WIDTH(...) \
	CompileTimeAssert< IsFixedWidthIntType<__VA_ARGS__>::Value > ERROR_Integer_Must_Have_Fixed_Width;


namespace dvrlib
{


	template < size_t Index, typename Integer >
	bool TestBit(Integer number)
	{
		TOOLKIT_MUST_HAVE_FIXED_WIDTH(Integer);
		return number & (1 << Index);
	}


	template< size_t Index, typename Integer >
	Integer SetBit(Integer number)
	{
		TOOLKIT_MUST_HAVE_FIXED_WIDTH(Integer);
		return number | (1 << Index);
	}


	template < size_t Index, typename Integer >
	Integer ResetBit(Integer number)
	{
		TOOLKIT_MUST_HAVE_FIXED_WIDTH(Integer);
		return number & ~(1 << Index);
	}


	template < size_t Index, typename Integer >
	Integer FlipBit(Integer number)
	{
		TOOLKIT_MUST_HAVE_FIXED_WIDTH(Integer);
		return number ^ (1 << Index);
	}


}


#endif
