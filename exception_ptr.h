#ifndef STINGRAYKIT_EXCEPTION_PTR_H
#define STINGRAYKIT_EXCEPTION_PTR_H

#include <stingray/toolkit/light_shared_ptr.h>
#include <stingray/toolkit/exception.h>

#include <exception>
#include <string>

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

	typedef light_shared_ptr<rethrown_exception> exception_ptr;

	inline exception_ptr make_exception_ptr(const std::exception &ex)
	{ return exception_ptr(new rethrown_exception(ex)); }

	inline void rethrow_exception(const exception_ptr& ex)
	{ if (ex) throw *ex; }

}

#endif

