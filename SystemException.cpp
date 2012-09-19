#include <stingray/toolkit/SystemException.h>

#include <errno.h>
#include <string.h>

#include <stingray/toolkit/StringUtils.h>

namespace stingray
{
	SystemException::SystemException(const std::string &message) throw():
		std::runtime_error(message + ": errno = " + ErrnoToStr(errno) + " (" + GetSystemError(errno) + ")") {}

	SystemException::SystemException(const std::string &message, int err) throw():
		std::runtime_error(message + ": errno = " + ErrnoToStr(err) + " (" + strerror(err) + ")") {}

	std::string SystemException::GetSystemError(int err) throw()
	{
		char buf[256];
		char *msg = strerror_r(err, buf, sizeof(buf));
		return msg? msg: "Unknown error";
	}

	std::string SystemException::GetSystemError() throw()
	{ return GetSystemError(errno); }

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
		default:
			return ToString(e);
		};
	}

#undef ERRNO_STR

}
