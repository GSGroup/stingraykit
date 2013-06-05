#ifndef STINGRAY_TOOLKIT_SOFTWAREVERSION_H
#define STINGRAY_TOOLKIT_SOFTWAREVERSION_H


#include <stingray/toolkit/StringUtils.h>
#include <stingray/toolkit/MemberListComparer.h>


namespace stingray
{

	class SoftwareVersion
	{
	private:
		size_t	_major;
		size_t	_minor;
		size_t	_build;

	public:
		explicit SoftwareVersion(size_t major = 0, size_t minor = 0, size_t build = 0)
			: _major(major), _minor(minor), _build(build)
		{ }

		std::string ToString() const
		{ return StringBuilder() % (int)_major % '.' % (int)_minor % '.' % (int)_build; }

		bool operator < (const SoftwareVersion& other) const
		{ return CompareMemberList<std::less>(&SoftwareVersion::_major, &SoftwareVersion::_minor, &SoftwareVersion::_build)(*this, other); }

		TOOLKIT_GENERATE_COMPARISON_OPERATORS_FROM_LESS(SoftwareVersion);
	};

}


#endif
