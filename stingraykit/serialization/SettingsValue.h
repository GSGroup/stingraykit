#ifndef SETTINGS_SETTINGS_VALUE_H__
#define SETTINGS_SETTINGS_VALUE_H__

// Copyright (c) 2011 - 2015, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

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

