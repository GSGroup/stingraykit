#ifndef STINGRAYKIT_PROGRESSVALUE_H
#define STINGRAYKIT_PROGRESSVALUE_H


#include <stingray/toolkit/Types.h>
#include <string>


namespace stingray
{


	struct ProgressValue
	{
		s64		Current;
		s64		Total;

		ProgressValue(s64 current = 0, s64 total = 0)
			: Current(current), Total(total)
		{ }

		int InPercents() const
		{ return Total != 0 ? 100 * Current / Total : 0; }

		std::string ToString() const;

		bool operator == (const ProgressValue& other) const { return Current == other.Current && Total == other.Total; }
		bool operator != (const ProgressValue& other) const { return !(*this == other); }
	};


}


#endif
