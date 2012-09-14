#ifndef STINGRAY_TOOLKIT_SYSTEMEXCEPTION_H
#define STINGRAY_TOOLKIT_SYSTEMEXCEPTION_H

#include <string>
#include <stdexcept>

namespace stingray
{

	struct SystemException : public std::runtime_error
	{
		SystemException(const std::string &message) throw();
		SystemException(const std::string &message, int err) throw();

		static std::string GetSystemError(int err) throw();

	private:
		std::string ErrnoToStr(int e);
	};
}

#endif
