#ifndef __GS_DVRLIB_TOOLKIT_PROGRESSVALUE_H__
#define __GS_DVRLIB_TOOLKIT_PROGRESSVALUE_H__


#include <stingray/toolkit/StringUtils.h>


namespace stingray
{


	struct ProgressValue
	{
		int		Current;
		int		Total;

		ProgressValue(int current, int total)
			: Current(current), Total(total)
		{ }

		size_t InPercents() const
		{ return Total != 0 ? 100 * Current / Total : 0; }

		std::string ToString() const
		{ return StringBuilder() % InPercents() % "%"; }
	};


}


#endif
