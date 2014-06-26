#ifndef STINGRAY_TOOLKIT_VERSION_H
#define STINGRAY_TOOLKIT_VERSION_H


#include <stingray/toolkit/MemberListComparer.h>
#include <stingray/toolkit/StringUtils.h>


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

		TOOLKIT_GENERATE_COMPARISON_OPERATORS_FROM_LESS(Version);
	};

}


#endif
