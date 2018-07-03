#ifndef STINGRAYKIT_PROGRESSVALUE_H
#define STINGRAYKIT_PROGRESSVALUE_H

// Copyright (c) 2011 - 2018, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <stingraykit/Types.h>
#include <string>


namespace stingray
{


	struct ProgressValue
	{
		s64		Current;
		s64		Total;

		ProgressValue(s64 current = 0, s64 total = 0)
			: Current(current), Total(total)
		{ }

		int InPercents() const
		{ return Total != 0 ? 100 * Current / Total : 0; }

		std::string ToString() const;

		bool operator <  (const ProgressValue& other) const { return InPercents() < other.InPercents(); }
		bool operator == (const ProgressValue& other) const { return Current == other.Current && Total == other.Total; }
		bool operator != (const ProgressValue& other) const { return !(*this == other); }
	};


}


#endif
