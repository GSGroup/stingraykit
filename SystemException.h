#ifndef STINGRAY_TOOLKIT_SYSTEM_EXCEPTION_H__
#define STINGRAY_TOOLKIT_SYSTEM_EXCEPTION_H__

#ifdef PLATFORM_POSIX
#	include <errno.h>
#	include <string.h>
#endif

#include <string>
#include <stdexcept>

#include <stingray/toolkit/StringUtils.h>

namespace stingray
{

#ifdef PLATFORM_POSIX
	struct SystemException : public std::runtime_error
	{
		static std::string GetSystemError() throw()			{ return strerror(errno); }
		static std::string GetSystemError(int err) throw()	{ return strerror(err); }
		SystemException(const std::string &message) throw(): std::runtime_error(message + ": errno = " + ErrnoToStr(errno) + " (" + GetSystemError() + ")") {}
		SystemException(const std::string &message, int err) throw(): std::runtime_error(message + ": errno = " + ErrnoToStr(err) + " (" + strerror(err) + ")") {}

	private:
#define ERRNO_STR(val) case val: return #val
		std::string ErrnoToStr(int e)
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
			ERRNO_STR(ENOTBLK);
			ERRNO_STR(ENOTDIR);
			ERRNO_STR(ENXIO);
			ERRNO_STR(EPERM);
			default:
				return ToString(e);
			};
		}
#undef ERRNO_STR
	};
#else
	typedef std::runtime_error SystemException;
#endif

}

#endif
