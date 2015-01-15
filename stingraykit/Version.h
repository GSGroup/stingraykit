#ifndef STINGRAYKIT_VERSION_H
#define STINGRAYKIT_VERSION_H


#include <stingraykit/compare/MemberListComparer.h>
#include <stingraykit/string/StringUtils.h>


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
