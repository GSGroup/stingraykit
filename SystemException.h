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

		static std::string GetErrorMessage(int err) throw();
		static std::string GetErrorMessage() throw();

		inline int GetErrorCode() const
		{ return _error; }

	private:
		int	_error;
		static std::string ErrnoToStr(int e);
	};


}

#endif
