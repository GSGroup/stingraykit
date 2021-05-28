#ifndef STINGRAYKIT_STRING_STRINGFORMAT_H
#define STINGRAYKIT_STRING_STRINGFORMAT_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/string/StringUtils.h>
#include <stingraykit/string/ToString.h>

namespace stingray
{

	namespace Detail
	{

		inline void ArgumentToString(string_ostream& result, size_t index, size_t width)
		{ STINGRAYKIT_THROW("Item index mismatch!"); }

		template < typename T0, typename... Ts >
		void ArgumentToString(string_ostream& result, size_t index, size_t width, const T0& p0, const Ts&... args)
		{
			if (index != 0)
				ArgumentToString(result, index - 1, width, args...);
			else if (width == 0)
				ToString(result, p0); //quick no width variant
			else
			{
				const std::string itemStr = ToString(p0);

				if (itemStr.size() < width)
					result << std::string(width - itemStr.size(), '0') << itemStr;
				else
					result << itemStr;
			}
		}

	}


	template < typename... Ts >
	std::string StringFormat(const std::string& format, const Ts&... args)
	{
		auto formatFragments = Split(format, "%");
		STINGRAYKIT_CHECK((Count(formatFragments) % 2) == 1, "Format mismatch: no corresponding %");

		string_ostream ss;

		for (size_t idx = 0; formatFragments.Valid(); ++idx, formatFragments.Next())
		{
			const StringRef fragment = *formatFragments;

			if ((idx % 2) == 0)
				ss << fragment.str();
			else if (fragment.empty())
				ss << "%";
			else
			{
				const size_t pos = fragment.find('$');
				const size_t index = FromString<size_t>(fragment.substr(0, pos));
				STINGRAYKIT_CHECK(index > 0, "Parameters indices must start from 1");
				const size_t width = (pos == std::string::npos) ? 0 : FromString<size_t>(fragment.substr(pos + 1));

				Detail::ArgumentToString(ss, index - 1, width, args...);
			}
		}

		return ss.str();
	}

}

#endif
