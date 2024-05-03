// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/Version.h>

#include <stingraykit/compare/MemberListComparer.h>
#include <stingraykit/string/StringParse.h>

namespace stingray
{

	bool Version::operator < (const Version& other) const
	{ return CompareMembersLess(&Version::_major, &Version::_minor, &Version::_build)(*this, other); }


	std::string Version::ToString() const
	{
		StringBuilder builder;
		builder % _major % '.' % _minor;
		if (_build)
			builder % '.' % _build;
		return builder;
	}


	Version Version::FromString(const std::string& str)
	{
		unsigned major;
		unsigned minor;
		optional<unsigned> build;
		STINGRAYKIT_CHECK(StringParse(str, "%1%.%2%.%3%", major, minor, build) || StringParse(str, "%1%.%2%", major, minor), FormatException(str));
		return Version(major, minor, build);
	}

}
