#ifndef STINGRAY_TOOLKIT_PROGRESSVALUE_H
#define STINGRAY_TOOLKIT_PROGRESSVALUE_H


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

		int InPercents() const
		{ return Total != 0 ? 100 * Current / Total : 0; }

		std::string ToString() const
		{ return StringBuilder() % InPercents() % "%"; }
	};


}


#endif
