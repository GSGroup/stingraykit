#ifndef STINGRAYKIT_THREAD_ITHREADENGINE_H
#define STINGRAYKIT_THREAD_ITHREADENGINE_H

// Copyright (c) 2011 - 2015, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/ICreator.h>
#include <stingraykit/diagnostics/ExternalAPIGuard.h>
#include <stingraykit/exception.h>
#include <stingraykit/shared_ptr.h>
#include <stingraykit/string/ToString.h>
#include <stingraykit/toolkit.h>

namespace stingray
{

	/**
	 * @addtogroup toolkit_threads
	 * @{
	 */

	struct ThreadSchedulingPolicy
	{
		STINGRAYKIT_ENUM_VALUES
		(
			NonRealtime,
			RealtimeRoundRobin,
			RealtimeFIFO
		);
		STINGRAYKIT_DECLARE_ENUM_CLASS(ThreadSchedulingPolicy);
	};


	struct ThreadSchedulingParams
	{
	private:
		ThreadSchedulingPolicy	_policy;
		int						_priority;

	public:
		ThreadSchedulingParams(ThreadSchedulingPolicy policy, int priority) :
			_policy(policy), _priority(priority)
		{ }

		ThreadSchedulingPolicy GetPolicy() const	{ return _policy; }
		int GetPriority() const						{ return _priority; }
	};


	struct ITLSUserData
	{
		virtual ~ITLSUserData() { }
	};
	STINGRAYKIT_DECLARE_PTR(ITLSUserData);


	struct ICancellationToken;


	class TLSData
	{
	private:
		const ICancellationToken*	_token;
		ExternalAPIGuardStack		_externalApiGuardStack;
		ITLSUserDataPtr				_userData;
		int							_syncPrimitiveCodeCounter;
		int							_disableCancellationCounter;

	public:
		explicit TLSData(const ITLSUserDataPtr& userData = null)
			: _token(NULL), _userData(userData), _syncPrimitiveCodeCounter(0), _disableCancellationCounter(0)
		{ }

		ExternalAPIGuardStack& GetExternalAPIGuardStack()				{ return _externalApiGuardStack; }
		const ExternalAPIGuardStack& GetExternalAPIGuardStack() const	{ return _externalApiGuardStack; }

		bool IsInSyncPrimitiveCode() const			{ return _syncPrimitiveCodeCounter > 0; }
		void EnterSyncPrimitiveCode()				{ ++_syncPrimitiveCodeCounter; }
		void LeaveSyncPrimitiveCode()				{ --_syncPrimitiveCodeCounter; }

		bool IsThreadCancellationEnabled() const	{ return _disableCancellationCounter <= 0; }
		void DisableThreadCancellation()			{ ++_disableCancellationCounter; }
		void EnableThreadCancellation()				{ --_disableCancellationCounter; }

		ITLSUserDataPtr GetUserData() const			{ return _userData; }

		void SetCancellationToken(const ICancellationToken* token)	{ STINGRAYKIT_CHECK(!token || (token && !_token), InvalidOperationException()); _token = token; }
		const ICancellationToken* GetCancellationToken()			{ return _token; }

		class DisableThreadCancellationToken
		{
			TLSData *				_tlsData;

		public:
			DisableThreadCancellationToken(TLSData * tlsData): _tlsData(tlsData)
			{ if (_tlsData) _tlsData->DisableThreadCancellation(); }
			~DisableThreadCancellationToken()
			{ if (_tlsData) _tlsData->EnableThreadCancellation(); }
		};
	};


	struct IThreadInfo
	{
		virtual ~IThreadInfo() { }

		virtual void RequestBacktrace() const = 0;
		virtual std::string GetName() const = 0;
	};
	STINGRAYKIT_DECLARE_PTR(IThreadInfo);


	struct IThread
	{
		typedef u64	ThreadId;

		virtual ~IThread() { }
		virtual void Interrupt() = 0;
		virtual ThreadId GetId() { STINGRAYKIT_THROW(NotImplementedException()); }
	};
	STINGRAYKIT_DECLARE_PTR(IThread);


	struct ThreadCpuStats
	{
	private:
		TimeDuration	_uTime;
		TimeDuration	_sTime;

	public:
		ThreadCpuStats()
		{ }

		ThreadCpuStats(TimeDuration uTime, TimeDuration sTime) : _uTime(uTime), _sTime(sTime)
		{ }

		ThreadCpuStats& operator += (const ThreadCpuStats& other)
		{
			_uTime += other._uTime;
			_sTime += other._sTime;
			return *this;
		}

		ThreadCpuStats operator + (const ThreadCpuStats& other)
		{
			ThreadCpuStats result(*this);
			return result += other;
		}

		TimeDuration GetUserTime() const	{ return _uTime; }
		TimeDuration GetSystemTime() const	{ return _sTime; }

		std::string ToString() const
		{ return StringBuilder() % "{ user: " % _uTime.GetMilliseconds() % " ms, system: " % _sTime.GetMilliseconds() % " ms }"; }
	};


	class ThreadStats
	{
	private:
		u64				_threadId;
		u64				_parentId;
		std::string		_threadName;

		ThreadCpuStats	_cpuStats;
		ThreadCpuStats	_childrenCpuStats;

	public:
		ThreadStats(u64 threadId, u64 parentId, const std::string& threadName, ThreadCpuStats cpuStats, ThreadCpuStats childrenCpuStats) :
			_threadId(threadId),
			_parentId(parentId),
			_threadName(threadName),
			_cpuStats(cpuStats),
			_childrenCpuStats(childrenCpuStats)
		{ }

		u64 GetThreadId() const						{ return _threadId; }
		u64 GetParentId() const						{ return _parentId; }
		std::string GetThreadName() const			{ return _threadName; }

		ThreadCpuStats GetCpuStats() const			{ return _cpuStats; }
		ThreadCpuStats GetChildrenCpuStats() const	{ return _childrenCpuStats; }
	};

	/** @} */

}


#endif
