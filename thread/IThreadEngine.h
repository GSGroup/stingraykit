#ifndef STINGRAY_TOOLKIT_THREAD_ITHREADENGINE_H
#define STINGRAY_TOOLKIT_THREAD_ITHREADENGINE_H


#include <stingray/toolkit/diagnostics/ExternalAPIGuard.h>
#include <stingray/toolkit/ICreator.h>
#include <stingray/toolkit/exception.h>
#include <stingray/toolkit/shared_ptr.h>
#include <stingray/toolkit/toolkit.h>


namespace stingray
{

	/**
	 * @addtogroup toolkit_threads
	 * @{
	 */

	struct ThreadSchedulingPolicy
	{
		TOOLKIT_ENUM_VALUES
		(
			NonRealtime,
			RealtimeRoundRobin,
			RealtimeFIFO
		);
		TOOLKIT_DECLARE_ENUM_CLASS(ThreadSchedulingPolicy);
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
	TOOLKIT_DECLARE_PTR(ITLSUserData);


	class ICancellationToken;


	class TLSData
	{
	private:
		std::string					_threadName;
		const ICancellationToken*	_token;
		ExternalAPIGuardStack		_externalApiGuardStack;
		ITLSUserDataPtr				_userData;
		int							_syncPrimitiveCodeCounter;

	public:
		explicit TLSData(const std::string& threadName, const ITLSUserDataPtr& userData = null)
			: _threadName(threadName), _token(NULL), _userData(userData), _syncPrimitiveCodeCounter(0)
		{ }

		ExternalAPIGuardStack& GetExternalAPIGuardStack()				{ return _externalApiGuardStack; }
		const ExternalAPIGuardStack& GetExternalAPIGuardStack() const	{ return _externalApiGuardStack; }

		bool IsInSyncPrimitiveCode() const			{ return _syncPrimitiveCodeCounter > 0; }
		void EnterSyncPrimitiveCode()				{ ++_syncPrimitiveCodeCounter; }
		void LeaveSyncPrimitiveCode()				{ --_syncPrimitiveCodeCounter; }

		ITLSUserDataPtr GetUserData() const			{ return _userData; }
		const std::string& GetThreadName() const	{ return _threadName; }
		void SetThreadName(const std::string& name)	{ _threadName = name; }

		void SetCancellationToken(const ICancellationToken* token)	{ TOOLKIT_CHECK(!token || (token && !_token), InvalidOperationException()); _token = token; }
		const ICancellationToken* GetCancellationToken()			{ return _token; }
	};


	struct IThreadInfo
	{
		virtual ~IThreadInfo() { }

		virtual void RequestBacktrace() const = 0;
		virtual std::string GetName() const = 0;
	};
	TOOLKIT_DECLARE_PTR(IThreadInfo);


	struct IThread
	{
		typedef u64	ThreadId;

		virtual ~IThread() { }
		virtual void Interrupt() = 0;
		virtual ThreadId GetId() { TOOLKIT_THROW(NotImplementedException()); }
	};
	TOOLKIT_DECLARE_PTR(IThread);


	class ThreadStats
	{
	private:
		u64				_threadId;
		u64				_parentId;
		std::string		_threadName;
		u64				_uTime;
		u64				_sTime;

		u64				_childrenUTime;
		u64				_childrenSTime;

	public:
		ThreadStats(u64 threadId, u64 parentId, const std::string& threadName, u64 uTime, u64 sTime, u64 childrenUTime, u64 childrenSTime) :
			_threadId(threadId),
			_parentId(parentId),
			_threadName(threadName),
			_uTime(uTime),
			_sTime(sTime),
			_childrenUTime(childrenUTime),
			_childrenSTime(childrenSTime)
		{ }

		u64 GetThreadId() const						{ return _threadId; }
		u64 GetParentId() const						{ return _parentId; }
		const std::string& GetThreadName() const	{ return _threadName; }
		u64 GetCpu() const							{ return _uTime + _sTime; }
		u64 GetUTime() const						{ return _uTime; }
		u64 GetSTime() const						{ return _sTime; }

		u64 GetChildrenUTime() const				{ return _childrenUTime; }
		u64 GetChildrenSTime() const				{ return _childrenSTime; }
	};

	/** @} */

}


#endif
