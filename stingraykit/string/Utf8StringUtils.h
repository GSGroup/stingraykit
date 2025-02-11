#ifndef STINGRAYKIT_STRING_UTF8STRINGUTILS_H
#define STINGRAYKIT_STRING_UTF8STRINGUTILS_H

// Copyright (c) 2011 - 2025, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/Range.h>
#include <stingraykit/string/Utf8IteratorRange.h>

namespace stingray
{

	inline size_t Utf8Strlen(string_view str)
	{ return Count(Utf8IteratorRange<string_view>(str)); }


	inline bool Utf8Validate(string_view str)
	{
		try
		{ Utf8Strlen(str); }
		catch (const std::exception& ex)
		{ return false; }

		return true;
	}


	template < bool IndexBySymbols >
	std::string Utf8Substring(string_view str, size_t pos, size_t count = string_view::npos)
	{
		Utf8IteratorRange<string_view> range(str);

		size_t index = 0;
		string_view::const_iterator start = range.GetIterator();
		string_view::const_iterator end = range.GetIterator();

		while ((IndexBySymbols ? index : (size_t)std::distance(start, end)) < pos)
		{
			++index;
			range.Next();
			end = range.GetIterator();
		}

		if (count == string_view::npos)
			return std::string(end, str.end());

		index = 0;
		start = end;

		while (range.Valid())
		{
			++index;
			range.Next();

			if ((IndexBySymbols ? index : (size_t)std::distance(start, range.GetIterator())) > count)
				break;

			end = range.GetIterator();
		}

		return std::string(start, end);
	}

}

#endif
