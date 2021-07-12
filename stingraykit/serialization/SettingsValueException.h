#ifndef STINGRAYKIT_SERIALIZATION_SETTINGSVALUEEXCEPTION_H
#define STINGRAYKIT_SERIALIZATION_SETTINGSVALUEEXCEPTION_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/exception.h>

namespace stingray
{

	/**
	 * @addtogroup core_settings
	 * @{
	 */

	class SettingsValueException : public Exception
	{
	private:
		std::string _backtrace;

	public:
		explicit SettingsValueException(const std::string& msg) : Exception(std::string()), _backtrace(msg) { }
		virtual ~SettingsValueException() noexcept { }

		virtual const char *what() const noexcept { return _backtrace.c_str(); }

		void Append(const std::string& property) { _backtrace = property + "->" + _backtrace; }
	};

	/** @} */

}

#endif
