#ifndef STINGRAYKIT_EXCEPTIONPTR_H
#define STINGRAYKIT_EXCEPTIONPTR_H

// Copyright (c) 2011 - 2025, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/shared_ptr.h>

namespace stingray
{

	class RethrownException : public std::exception
	{
		std::string _what;

	public:
		explicit RethrownException(const std::exception& ex) noexcept : _what(diagnostic_information(ex)) { }
		virtual ~RethrownException() noexcept { }

		virtual const char* what() const noexcept { return _what.c_str(); }
	};

	typedef shared_ptr<RethrownException> ExceptionPtr;

	inline ExceptionPtr MakeExceptionPtr(const std::exception& ex)
	{ return make_shared_ptr<RethrownException>(ex); }

	inline void RethrowException(const ExceptionPtr& ex)
	{ if (ex) STINGRAYKIT_THROW(*ex); }

}

#endif

