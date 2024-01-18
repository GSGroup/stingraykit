// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/string/HumanReadableSize.h>
#include <stingraykit/string/ToString.h>

#include <regex>

namespace stingray
{

	namespace
	{

		const std::string Suffixes(R"(kMGTPE)");
		const std::regex FromHumanRegex(R"((\d+)([)" + Suffixes + R"(]))");

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
		try
		{ return FromString<u64>(str); }
		catch (const std::exception&)
		{ }

		std::smatch match;
		if (std::regex_match(str, match, FromHumanRegex))
		{
			const u64 num = FromString<u64>(match.str(1));
			const char suffix = *match[2].first;

			for (size_t i = 0; i < Suffixes.size(); ++i)
				if (suffix == Suffixes[i])
					return num * ((u64)1 << (10 * (i + 1)));
		}

		STINGRAYKIT_THROW(FormatException(str));
	}

}
