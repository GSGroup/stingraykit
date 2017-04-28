#include <stingraykit/string/HumanReadableSize.h>
#include <stingraykit/string/ToString.h>
#include <stingraykit/string/regex.h>

namespace stingray
{

	namespace
	{

		const std::string Suffixes("kMGTPEZ");

	}


	std::string ToHumanReadableSize(u64 size)
	{
		for (size_t i = 0; i < Suffixes.size(); ++i)
		{
			u64 classToCompare = (u64)1 << 10 * (i + 1);
			u64 remainder = size % classToCompare;
			size_t classModifier = remainder > 0 ? 1 : 0;
			size_t roundModifier = remainder >= classToCompare / 2 ? 1 : 0;
			if (size < (u64)1 << 10 * (i + 1 + classModifier))
			{
				u64 result = size / ((u64)1 << 10 * i) + roundModifier;
				if (i > 0)
					return ToString(result) + Suffixes[i - 1];
				else
					return ToString(result);
			}
		}

		return ToString(size);
	}


	u64 FromHumanReadableSize(const std::string& str)
	{
		u64 num;
		try
		{
			num = FromString<u64>(str);
			return num;
		}
		catch (const std::exception &) { }

		smatch m;
		std::string suffix;
		if (regex_search(str, m, regex(std::string("^(\\d+)") + "([" + Suffixes + "])$")))
		{
			num = FromString<u64>(m[1]);
			suffix = FromString<std::string>(m[2]);

			for (size_t i = 0; i < Suffixes.size(); ++i)
				if (suffix[0] == Suffixes[i])
					return num * ((u64)1 << 10 * (i + 1));
		}

		STINGRAYKIT_THROW(FormatException(str));
	}

}
