#ifndef STINGRAY_TOOLKIT_SOFTWAREVERSION_H
#define STINGRAY_TOOLKIT_SOFTWAREVERSION_H


#include <stingray/toolkit/MemberListComparer.h>
#include <stingray/toolkit/StringUtils.h>


namespace stingray
{

	class SoftwareVersion
	{
	private:
		unsigned	_major;
		unsigned	_minor;
		unsigned	_build;

	public:
		SoftwareVersion() : _major(), _minor(), _build() { }
		SoftwareVersion(unsigned major, unsigned minor, unsigned build)
			: _major(major), _minor(minor), _build(build)
		{ }

		static SoftwareVersion FromString(const std::string& version);

		std::string ToString() const
		{ return StringBuilder() % _major % '.' % _minor % '.' % _build; }

		bool operator < (const SoftwareVersion& other) const
		{ return CompareMemberList<std::less>(&SoftwareVersion::_major, &SoftwareVersion::_minor, &SoftwareVersion::_build)(*this, other); }

		TOOLKIT_GENERATE_COMPARISON_OPERATORS_FROM_LESS(SoftwareVersion);
	};

}


#endif
