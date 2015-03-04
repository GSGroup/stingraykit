#ifndef STINGRAYKIT_VERSION_H
#define STINGRAYKIT_VERSION_H

// Copyright (c) 2011 - 2015, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <stingraykit/compare/MemberListComparer.h>
#include <stingraykit/string/ToString.h>


namespace stingray
{

	class Version
	{
	private:
		unsigned	_major;
		unsigned	_minor;
		unsigned	_build;

	public:
		Version() : _major(), _minor(), _build() { }
		Version(unsigned major, unsigned minor, unsigned build)
			: _major(major), _minor(minor), _build(build)
		{ }

		static Version FromString(const std::string& version);

		std::string ToString() const
		{ return StringBuilder() % _major % '.' % _minor % '.' % _build; }

		bool operator < (const Version& other) const
		{ return CompareMembersLess(&Version::_major, &Version::_minor, &Version::_build)(*this, other); }

		STINGRAYKIT_GENERATE_COMPARISON_OPERATORS_FROM_LESS(Version);
	};

}


#endif
