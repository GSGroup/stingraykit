#ifndef STINGRAYKIT_SERIALIZATION_SETTINGSVALUEEXCEPTION_H
#define STINGRAYKIT_SERIALIZATION_SETTINGSVALUEEXCEPTION_H

#include <string>
#include <stingray/toolkit/exception.h>

namespace stingray
{

	/**
	 * @addtogroup core_settings
	 * @{
	 */

	class SettingsValueException : public Exception
	{
		std::string _backtrace;
	public:
		SettingsValueException(const std::string &msg): Exception(std::string()), _backtrace(msg) {}
		~SettingsValueException() throw() {}

		void Append(const std::string &property) { _backtrace = property + "->" + _backtrace; }
		const char *what() const throw() { return _backtrace.c_str(); }
	};

	/** @} */

}

#endif
