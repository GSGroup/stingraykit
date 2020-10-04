#ifndef STINGRAYKIT_SYSTEMEXCEPTION_H
#define STINGRAYKIT_SYSTEMEXCEPTION_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/exception.h>

namespace stingray
{

	class SystemException : public Exception
	{
	private:
		int	_error;

	public:
		SystemException(const std::string& message) noexcept;
		SystemException(const std::string& message, int err) noexcept;
		SystemException(const std::string& message, const std::string& path, int err) noexcept;
		virtual ~SystemException() noexcept { }

		int GetErrorCode() const { return _error; }

		static std::string GetErrorMessage(int err) noexcept;
		static std::string GetErrorMessage() noexcept;

	private:
		static std::string ErrnoToStr(int e);
	};

#define STINGRAYKIT_THROW_SYSTEM_EXCEPTION(MESSAGE, PATH, ERROR) \
	do \
	{ \
		switch(ERROR) \
		{ \
		case ENOENT:	STINGRAYKIT_THROW(FileNotFoundException(PATH)); \
		case EACCES:	STINGRAYKIT_THROW(AccessDeniedException(MESSAGE)); \
		case EIO:		STINGRAYKIT_THROW(InputOutputException(MESSAGE)); \
		case EBUSY:		STINGRAYKIT_THROW(DeviceBusyException(MESSAGE)); \
		case ENOSPC:	STINGRAYKIT_THROW(NoSpaceLeftException(MESSAGE)); \
		case EAGAIN:	STINGRAYKIT_THROW(ResourceUnavailableException(MESSAGE)); \
		default:		STINGRAYKIT_THROW(SystemException(MESSAGE, PATH, ERROR)); \
		} \
	} while(false)

}

#endif
