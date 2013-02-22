#ifndef STINGRAY_TOOLKIT_PROGRESSVALUE_H
#define STINGRAY_TOOLKIT_PROGRESSVALUE_H


#include <stingray/toolkit/StringUtils.h>


namespace stingray
{


	struct ProgressValue
	{
		s64		Current;
		s64		Total;

		ProgressValue(int current, int total)
			: Current(current), Total(total)
		{ }

		int InPercents() const
		{ return Total != 0 ? 100 * Current / Total : 0; }

		std::string ToString() const
		{ return StringBuilder() % InPercents() % "%"; }

		bool operator == (const ProgressValue& other) const { return Current == other.Current && Total == other.Total; }
		bool operator != (const ProgressValue& other) const { return !(*this == other); }
	};


}


#endif
