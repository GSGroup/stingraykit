#ifndef STINGRAY_TOOLKIT_SERIALIZATION_SETTINGSVALUEFORWARD_H
#define STINGRAY_TOOLKIT_SERIALIZATION_SETTINGSVALUEFORWARD_H

#include <map>
#include <vector>
#include <stingray/toolkit/self_counter.h>

namespace stingray
{
	class SettingsValue;
	STINGRAYKIT_DECLARE_SELF_COUNT_PTR(SettingsValue);

	typedef std::map<const std::string, SettingsValueSelfCountPtr> SettingsValueMap;
	typedef std::vector<SettingsValueSelfCountPtr> SettingsValueList;

}

#endif
