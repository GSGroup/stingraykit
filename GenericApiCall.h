#ifndef STINGRAY_TOOLKIT_GENERICAPICALL_H
#define STINGRAY_TOOLKIT_GENERICAPICALL_H

#include <stingray/toolkit/log/Logger.h>
#include <stingray/threads/Thread.h>
#include <stingray/toolkit/exception.h>

#define GENERIC_API_CALL(ReturnType, SuccessValue, ...) \
	do \
	{ \
		ExternalAPIGuards::EnterGuard __g__(#__VA_ARGS__); \
		const ReturnType res = __VA_ARGS__; \
		if (res != SuccessValue) \
		{ \
			DETAIL_DECLARE_STATIC_LOGGER_ACCESSOR; \
			std::string msg = #__VA_ARGS__ " failed, result = " + ToString(res); \
			TOOLKIT_STATIC_LOGGER.Error() << msg; \
			TOOLKIT_THROW(std::runtime_error(msg)); \
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
			TOOLKIT_STATIC_LOGGER.Error() << #__VA_ARGS__ " failed, result = " << (int)res; \
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

#define GENERIC_API_CALL_TRANSPARENT(...) (::stingray::Detail::GenericApiCallGuard(#__VA_ARGS__) ? (__VA_ARGS__) : (__VA_ARGS__))

#endif

