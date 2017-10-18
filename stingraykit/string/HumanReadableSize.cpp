#include <stingraykit/string/HumanReadableSize.h>
#include <stingraykit/string/ToString.h>
#include <stingraykit/string/regex.h>

namespace stingray
{

	namespace
	{

		const std::string Suffixes("kMGTPE");
		const regex FromHumanRegexp(std::string("^(\\d+)") + "([" + Suffixes + "])$");

	}


	std::string ToHumanReadableSize(u64 size, float allowedErrorFactor, bool whitespaceBeforeSuffix)
	{
		STINGRAYKIT_CHECK(allowedErrorFactor <= 0.2, LogicException("Unreasonable allowedErrorFactor"));

		const std::string whitespace = whitespaceBeforeSuffix ? " " : std::string();

		if (size == 0)
			return "0" + whitespace;

		for (int i = Suffixes.size(); i >= 0; --i)
		{
			u64 measUnit = (u64)1 << (10 * i);
			u64 roundedInUnits = size / measUnit + (measUnit > 1 && size % measUnit >= measUnit / 2);
			u64 rounded = roundedInUnits * measUnit;
			u64 delta = size * allowedErrorFactor;

			if (rounded >= size - delta && rounded <= size + delta)
			{
				const std::string sizeString = ToString(roundedInUnits) + whitespace;
				return i > 0 ? sizeString + Suffixes[i - 1] : sizeString;
			}
		}

		STINGRAYKIT_THROW(LogicException("Conversion failed"));
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
		if (regex_search(str, m, FromHumanRegexp))
		{
			num = FromString<u64>(m[1]);
			const std::string suffix = FromString<std::string>(m[2]);

			for (size_t i = 0; i < Suffixes.size(); ++i)
				if (suffix[0] == Suffixes[i])
					return num * ((u64)1 << 10 * (i + 1));
		}

		STINGRAYKIT_THROW(FormatException(str));
	}

}
