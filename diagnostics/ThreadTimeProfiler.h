#ifndef STINGRAY_TOOLKIT_DIAGNOSTICS_THREADTIMEPROFILER_H
#define STINGRAY_TOOLKIT_DIAGNOSTICS_THREADTIMEPROFILER_H


#include <stingray/toolkit/time/ElapsedTime.h>
#include <stingray/toolkit/NestedTypeCheck.h>


namespace stingray
{

	/**
	 * @addtogroup toolkit_profiling
	 * @{
	 */

	TOOLKIT_DECLARE_METHOD_CHECK(GetThreadMicroseconds);


	template < typename ThreadEngineType, bool HasGetThreadMicroseconds = HasMethod_GetThreadMicroseconds<ThreadEngineType>::Value >
	class BasicThreadTimeProfiler
	{
	private:
		s64		_start;

	public:
		BasicThreadTimeProfiler()
			: _start(ThreadEngineType::GetThreadMicroseconds())
		{ }

		s64 GetMicroseconds() const { return (s64)ThreadEngineType::GetThreadMicroseconds() - _start; }
	};


	template < typename ThreadEngineType >
	class BasicThreadTimeProfiler<ThreadEngineType, false>
	{
	private:
		ElapsedTime		_timer;

	public:
		BasicThreadTimeProfiler()
		{ }

		s64 GetMicroseconds() const { return _timer.ElapsedMicroseconds(); }
	};


	typedef BasicThreadTimeProfiler<ThreadEngine>		ThreadTimeProfiler;

	/** @} */

}


#endif
