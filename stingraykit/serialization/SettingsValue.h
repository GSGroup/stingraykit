#ifndef SETTINGS_SETTINGS_VALUE_H__
#define SETTINGS_SETTINGS_VALUE_H__

#include <stingraykit/serialization/SettingsValueForward.h>
#include <stingraykit/serialization/SettingsValueException.h>
#include <stingraykit/self_counter.h>
#include <stingraykit/toolkit.h>
#include <stingraykit/variant.h>

namespace stingray
{

	/**
	 * @addtogroup core_settings
	 * @{
	 */
	typedef TypeList_8<EmptyType, bool, s64, double, std::string, std::vector<u8>, SettingsValueList, SettingsValueMap> SettingsValueTypes;

	class SettingsValue : public self_counter<SettingsValue>, public variant<SettingsValueTypes>
	{
		STINGRAYKIT_NONCOPYABLE(SettingsValue);

	public:
		SettingsValue()
		{ }

		template < typename T >
		SettingsValue(const T &val): variant<SettingsValueTypes>(val)
		{ }
	};

	/** @} */

}

#endif

