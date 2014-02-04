#include <stingray/toolkit/SystemException.h>

#include <errno.h>
#include <string.h>

#include <stingray/toolkit/StringUtils.h>

namespace stingray
{
	SystemException::SystemException(const std::string &message) throw():
		std::runtime_error(message + ": errno = " + GetErrorMessage(errno)), _error(errno) {}

	SystemException::SystemException(const std::string &message, int err) throw():
		std::runtime_error(message + ": errno = " + GetErrorMessage(err)), _error(err) {}

	std::string SystemException::GetErrorMessage(int err) throw()
	{
		std::string result = ErrnoToStr(err);
		char buf[256];
		char *msg = strerror_r(err, buf, sizeof(buf));
		result += " (";
		result += msg ? msg: "Unknown error";
		result += ")";
		return result;
	}

	std::string SystemException::GetErrorMessage() throw()
	{ return GetErrorMessage(errno); }

#define ERRNO_STR(val) case val: return #val
	std::string SystemException::ErrnoToStr(int e)
	{
		switch (e)
		{
		ERRNO_STR(EACCES);
		ERRNO_STR(EBUSY);
		ERRNO_STR(EFAULT);
		ERRNO_STR(EINVAL);
		ERRNO_STR(ELOOP);
		ERRNO_STR(EMFILE);
		ERRNO_STR(ENAMETOOLONG);
		ERRNO_STR(ENODEV);
		ERRNO_STR(ENOENT);
		ERRNO_STR(ENOMEM);
		ERRNO_STR(ENOTDIR);
		ERRNO_STR(ENXIO);
		ERRNO_STR(EPERM);
		ERRNO_STR(EBADF);
		ERRNO_STR(EINTR);
		ERRNO_STR(ECHILD);
		default:
			return ToString(e);
		};
	}

#undef ERRNO_STR

}
