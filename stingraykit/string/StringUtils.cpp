// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/string/StringUtils.h>

namespace stingray
{

	string_view::size_type EditDistance(string_view s1, string_view s2)
	{
		if (s1 == s2)
			return 0;

		if (s1.empty())
			return s2.size();

		if (s2.empty())
			return s1.size();

		std::vector<string_view::size_type> v0(s2.size() + 1);
		std::vector<string_view::size_type> v1(s2.size() + 1);

		for (string_view::size_type i = 0; i < v0.size(); ++i)
			v0[i] = i;

		for (string_view::size_type i = 0; i < s1.size(); ++i)
		{
			v1[0] = i + 1;

			for (string_view::size_type j = 0; j < s2.size(); ++j)
				v1[j + 1] = std::min(std::min(v1[j] + 1, v0[j + 1] + 1), v0[j] + (s1[i] == s2[j] ? 0 : 1));

			v0 = v1;
		}

		return v1[s2.size()];
	}

}
