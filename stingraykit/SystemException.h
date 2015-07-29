#ifndef STINGRAYKIT_SYSTEMEXCEPTION_H
#define STINGRAYKIT_SYSTEMEXCEPTION_H

// Copyright (c) 2011 - 2015, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/exception.h>

#include <string>
#include <stdexcept>

namespace stingray
{

	struct SystemException : public Exception
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

#define STINGRAYKIT_THROW_SYSTEM_EXCEPTION(MESSAGE, ERROR) \
	do \
	{ \
		switch(ERROR) \
		{ \
		case ENOENT:	STINGRAYKIT_THROW(FileNotFoundException(MESSAGE)); \
		case EACCES:	STINGRAYKIT_THROW(AccessDeniedException(MESSAGE)); \
		case EIO:		STINGRAYKIT_THROW(InputOutputException(MESSAGE)); \
		case EBUSY:		STINGRAYKIT_THROW(DeviceBusyException(MESSAGE)); \
		case ENOSPC:	STINGRAYKIT_THROW(NoSpaceLeftException(MESSAGE)); \
		default:		STINGRAYKIT_THROW(SystemException(MESSAGE, ERROR)); \
		} \
	} while(false)

}

#endif
