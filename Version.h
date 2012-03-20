#ifndef __GS_STINGRAY_TOOLKIT_VERSION_H__
#define __GS_STINGRAY_TOOLKIT_VERSION_H__


#include <stingray/toolkit/StringUtils.h>
#include <stingray/toolkit/MemberListComparer.h>


namespace stingray
{

	class Version
	{
	private:
		size_t	_major;
		size_t	_minor;
		size_t	_build;

	public:
		explicit Version(size_t major = 0, size_t minor = 0, size_t build = 0)
			: _major(major), _minor(minor), _build(build)
		{ }

		std::string ToString() const
		{ return StringBuilder() % (int)_major % '.' % (int)_minor % '.' % (int)_build; }

		bool operator < (const Version& other) const
		{ return CompareMemberList<std::less>(&Version::_major, &Version::_minor, &Version::_build)(*this, other); }

		TOOLKIT_GENERATE_COMPARISON_OPERATORS_FROM_LESS(Version);
	};

}


#endif
