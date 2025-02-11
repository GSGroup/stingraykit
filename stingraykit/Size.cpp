// Copyright (c) 2011 - 2025, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/Size.h>

#include <stingraykit/compare/MemberListComparer.h>
#include <stingraykit/string/ToString.h>

namespace stingray
{

	bool Size::operator < (const Size& other) const
	{ return CompareMembersLess(&Size::Width, &Size::Height)(*this, other); }


	std::string Size::ToString() const
	{ return StringBuilder() % Width % "x" % Height; }


	bool SizeScale::operator < (const SizeScale& other) const
	{ return CompareMembersLess(&SizeScale::WidthScale, &SizeScale::HeightScale)(*this, other); }


	std::string SizeScale::ToString() const
	{ return StringBuilder() % WidthScale % ":" % HeightScale; }

}
