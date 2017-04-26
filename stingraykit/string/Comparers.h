#ifndef STINGRAYKIT_STRING_COMPARERS_H
#define STINGRAYKIT_STRING_COMPARERS_H

// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/compare/comparers.h>

namespace stingray
{

	namespace Detail
	{

		template < class Comparer >
		class CaseInsensitiveCompare
		{
		private:
			Comparer	_comparer;

		public:
			explicit CaseInsensitiveCompare(Comparer comparer = Comparer())
				: _comparer(comparer)
			{ }

			bool operator()(char first, char second) const
			{
				return _comparer(std::tolower(first), std::tolower(second));
			}
		};

	}


	class CaseInsensitiveLess
	{
	public:
		bool operator()(const std::string& first, const std::string& second) const
		{
			return std::lexicographical_compare(first.begin(), first.end(), second.begin(), second.end(), Detail::CaseInsensitiveCompare<comparers::Less>());
		}
	};


	class CaseInsensitiveEquals
	{
	public:
		bool operator()(const std::string& first, const std::string& second) const
		{
			return first.length() == second.length() && std::equal(first.begin(), first.end(), second.begin(), Detail::CaseInsensitiveCompare<comparers::Equals>());
		}
	};

}

#endif
