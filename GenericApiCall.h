#ifndef STINGRAY_TOOLKIT_GENERICAPICALL_H
#define STINGRAY_TOOLKIT_GENERICAPICALL_H

#include <stingray/log/Logger.h>
#include <stingray/toolkit/exception.h>

#define GENERIC_API_CALL(ReturnType, SuccessValue, ...) \
	do \
	{ \
		ExternalAPIGuards::EnterGuard __g__(#__VA_ARGS__); \
		const ReturnType res = __VA_ARGS__; \
		if (res != SuccessValue) \
		{ \
			std::string msg = #__VA_ARGS__ " failed, result = " + ToString(res); \
			s_logger.Error() << msg; \
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
			Logger::Error() << #__VA_ARGS__ " failed, result = " << (int)res; \
		} \
	} while (0)

#endif

