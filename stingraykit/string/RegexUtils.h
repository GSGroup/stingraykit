#ifndef STINGRAYKIT_STRING_REGEXUTILS_H
#define STINGRAYKIT_STRING_REGEXUTILS_H

// Copyright (c) 2011 - 2025, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/string/string_view.h>
#include <stingraykit/Types.h>

#include <regex>

namespace stingray
{

	std::string GenerateNumericRangeRegex(u64 start, u64 end, bool match);


	using svmatch = std::match_results<string_view::const_iterator>;


	inline string_view svmatch_str(const svmatch& match, svmatch::size_type index = 0)
	{
		const auto& subMatch = match[index];
		return subMatch.matched ? string_view(&*subMatch.first, std::distance(subMatch.first, subMatch.second)) : string_view();
	}


	inline string_view smatch_str(const std::smatch& match, std::smatch::size_type index = 0)
	{
		const auto& subMatch = match[index];
		return subMatch.matched ? string_view(&*subMatch.first, std::distance(subMatch.first, subMatch.second)) : string_view();
	}

}

#endif
