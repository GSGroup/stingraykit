#ifndef STINGRAYKIT_EXCEPTION_PTR_H
#define STINGRAYKIT_EXCEPTION_PTR_H

// Copyright (c) 2011 - 2018, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <exception>
#include <string>

#include <stingraykit/shared_ptr.h>
#include <stingraykit/exception.h>

namespace stingray
{

	class rethrown_exception : public std::exception
	{
		std::string _what;

	public:
		rethrown_exception(const std::exception &ex) throw() : _what(diagnostic_information(ex)) {}
		virtual ~rethrown_exception() throw() {}

		virtual const char* what() const throw() { return _what.c_str(); }
	};

	typedef shared_ptr<rethrown_exception> exception_ptr;

	inline exception_ptr make_exception_ptr(const std::exception &ex)
	{ return make_shared<rethrown_exception>(ex); }

	inline void rethrow_exception(const exception_ptr& ex)
	{ if (ex) throw *ex; }

}

#endif

