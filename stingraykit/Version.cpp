// Copyright (c) 2011 - 2015, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/Version.h>

#include <stdio.h>

namespace stingray
{

	Version Version::FromString(const std::string& version)
	{
		unsigned major, minor, build;
		const int parsed = sscanf(version.c_str(), "%u.%u.%u", &major, &minor, &build);
		STINGRAYKIT_CHECK(parsed == 3 || parsed == 2, FormatException(version));
		return Version(major, minor, parsed == 3 ? build : optional<unsigned>());
	}

	std::string Version::ToString() const
	{
		StringBuilder builder = StringBuilder() % _major % '.' % _minor;
		if (_build)
			builder % '.' % *_build;
		return builder;
	}

}
