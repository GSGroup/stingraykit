#include <stingraykit/string/StringUtils.h>

namespace stingray
{

	std::string::size_type EditDistance(const std::string& s1, const std::string& s2)
	{
		if (s1 == s2)
			return 0;

		if (s1.empty())
			return s2.size();

		if (s2.empty())
			return s1.size();

		std::vector<std::string::size_type> v0(s2.size() + 1);
		std::vector<std::string::size_type> v1(s2.size() + 1);

		for (std::string::size_type i = 0; i < v0.size(); ++i)
			v0[i] = i;

		for (std::string::size_type i = 0; i < s1.size(); ++i)
		{
			v1[0] = i + 1;

			for (std::string::size_type j = 0; j < s2.size(); ++j)
				v1[j + 1] = std::min(std::min(v1[j] + 1, v0[j + 1] + 1), v0[j] + (s1[i] == s2[j] ? 0 : 1));

			v0 = v1;
		}

		return v1[s2.size()];
	}

}
