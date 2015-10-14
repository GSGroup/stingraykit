#ifndef STINGRAYKIT_DIAGNOSTICS_GENERICAPICALL_H
#define STINGRAYKIT_DIAGNOSTICS_GENERICAPICALL_H

// Copyright (c) 2011 - 2015, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/log/Logger.h>
#include <stingraykit/thread/Thread.h>
#include <stingraykit/exception.h>

#define GENERIC_API_CALL(ReturnType, SuccessValue, ...) \
	do \
	{ \
		ExternalAPIGuards::EnterGuard __g__(#__VA_ARGS__); \
		const ReturnType res = __VA_ARGS__; \
		if (res != SuccessValue) \
		{ \
			DETAIL_DECLARE_STATIC_LOGGER_ACCESSOR; \
			std::string msg = #__VA_ARGS__ " failed, result = " + ToString(res); \
			STINGRAYKIT_STATIC_LOGGER.Error() << msg; \
			STINGRAYKIT_THROW(std::runtime_error(msg)); \
		} \
	} while (0)

#define GENERIC_API_CALL_NO_THROW(ReturnType, SuccessValue, ...) \
	do \
	{ \
		ExternalAPIGuards::EnterGuard __g__(#__VA_ARGS__); \
		const ReturnType res = __VA_ARGS__; \
		if (res != SuccessValue) \
		{ \
			DETAIL_DECLARE_STATIC_LOGGER_ACCESSOR; \
			STINGRAYKIT_STATIC_LOGGER.Error() << #__VA_ARGS__ " failed, result = " << ToString(res); \
		} \
	} while (0)


namespace stingray {
namespace Detail
{

	class GenericApiCallGuard
	{
		ExternalAPIGuards::EnterGuard	_externalApiGuard;

	public:
		GenericApiCallGuard(const char* call) : _externalApiGuard(call) { }
		operator bool() const { return true; }
	};

}}

#define GENERIC_API_CALL_TRANSPARENT(...) (::stingray::Detail::GenericApiCallGuard(#__VA_ARGS__) ? (__VA_ARGS__) : (STINGRAYKIT_THROW("Should never get here!")))

#endif

