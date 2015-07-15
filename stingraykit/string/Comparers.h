#ifndef STINGRAYKIT_STRING_COMPARERS_H
#define STINGRAYKIT_STRING_COMPARERS_H

#include <algorithm>
#include <cctype>

namespace stingray
{

	class CaseInsensitiveLess
	{
	public:
		bool operator()(const std::string& first, const std::string& second) const
		{
			return std::lexicographical_compare(first.begin(), first.end(), second.begin(), second.end(), Compare);
		}

	private:
		static bool Compare(char first, char second)
		{
			return std::tolower(first) < std::tolower(second);
		}
	};

}

#endif
