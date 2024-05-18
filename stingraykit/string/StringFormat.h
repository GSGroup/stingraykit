#ifndef STINGRAYKIT_STRING_STRINGFORMAT_H
#define STINGRAYKIT_STRING_STRINGFORMAT_H

// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

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
	std::string StringFormat(const string_view& format, const Ts&... args)
	{
		string_ostream result;

		for (size_t pos = 0; pos < format.size(); ++pos)
		{
			const char c = format[pos];

			switch (c)
			{
			case '%':
			{
				const size_t nextPercentPos = format.find('%', pos + 1);
				STINGRAYKIT_CHECK(nextPercentPos != string_view::npos, FormatException(format.copy()));

				const string_view argIndexStr = format.substr(pos + 1, nextPercentPos - pos - 1);
				if (argIndexStr.empty())
				{
					result << '%';
					++pos;
					break;
				}

				const size_t widthPos = argIndexStr.find('$');

				const string_view indexStr = argIndexStr.substr(0, widthPos);
				STINGRAYKIT_CHECK(!indexStr.empty(), FormatException(format.copy()));

				const size_t index = FromString<size_t>(indexStr);
				STINGRAYKIT_CHECK(index > 0, FormatException(format.copy()));

				size_t width = 0;
				if (widthPos != std::string::npos)
				{
					const string_view widthStr = argIndexStr.substr(widthPos + 1);
					STINGRAYKIT_CHECK(!widthStr.empty(), FormatException(format.copy()));

					width = FromString<size_t>(widthStr);
				}

				Detail::ArgumentToString(result, index - 1, width, args...);
				pos += argIndexStr.size() + 1;
				break;
			}

			default:
				result << c;
				break;
			}
		}

		return result.str();
	}

}

#endif
