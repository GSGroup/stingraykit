#ifndef STINGRAYKIT_THREAD_ICANCELLATIONTOKEN_H
#define STINGRAYKIT_THREAD_ICANCELLATIONTOKEN_H


#include <stingraykit/time/Time.h>
#include <stingraykit/safe_bool.h>

namespace stingray
{

	struct ICancellationHandler
	{
	protected:
		~ICancellationHandler() { }

	public:
		virtual void Cancel() = 0;
	};


	class CancellationHandlerHolderBase;


	struct ICancellationToken : public safe_bool<ICancellationToken>
	{
	private:
		friend class CancellationHandlerHolderBase;

	protected:
		virtual ~ICancellationToken() { }

	public:
		virtual void Cancel() = 0;
		virtual void Reset() = 0;

		virtual void Sleep(TimeDuration duration) const = 0;

		virtual bool IsCancelled() const = 0;

		bool boolean_test() const { return !IsCancelled(); }

	protected:
		virtual bool RegisterCancellationHandler(ICancellationHandler& handler) const = 0;
		virtual bool TryUnregisterCancellationHandler() const = 0;
		virtual void UnregisterCancellationHandler() const = 0;
	};


	class CancellationHandlerHolderBase
	{
	private:
		const ICancellationToken&	_token;
		bool						_cancelledBeforeRegistration;

	public:
		bool IsCancelled() const
		{ return _cancelledBeforeRegistration; }

	protected:
		CancellationHandlerHolderBase(const ICancellationToken& token) :
			_token(token), _cancelledBeforeRegistration(false)
		{ }

		~CancellationHandlerHolderBase()
		{ }

		void Register(ICancellationHandler& handler)
		{ _cancelledBeforeRegistration = !_token.RegisterCancellationHandler(handler); }

		bool TryUnregister()
		{ return _token.TryUnregisterCancellationHandler(); }

		void Unregister()
		{ _token.UnregisterCancellationHandler(); }
	};


	template<typename CancellationHandler_>
	class CancellationHandlerHolder : public CancellationHandlerHolderBase
	{
		STINGRAYKIT_NONCOPYABLE(CancellationHandlerHolder);

	private:
		CancellationHandler_	_handler;

	public:
		CancellationHandlerHolder(const ICancellationToken& token) :
			CancellationHandlerHolderBase(token)
		{ Register(_handler); }

		template<typename T1>
		CancellationHandlerHolder(const T1& p1, const ICancellationToken& token) :
			CancellationHandlerHolderBase(token), _handler(p1)
		{ Register(_handler); }

		template<typename T1, typename T2>
		CancellationHandlerHolder(const T1& p1, const T2& p2, const ICancellationToken& token) :
			CancellationHandlerHolderBase(token), _handler(p1, p2)
		{ Register(_handler); }

		~CancellationHandlerHolder()
		{ Unregister(); }

		CancellationHandler_& GetHandler()
		{ return _handler; }
	};

}

#endif
