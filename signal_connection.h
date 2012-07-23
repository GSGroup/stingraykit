#ifndef __GS_DVRLIB_TOOLKIT_SIGNAL_CONNECTION_H__
#define __GS_DVRLIB_TOOLKIT_SIGNAL_CONNECTION_H__

#include <vector>
#include <algorithm>
#include <stingray/toolkit/ITaskExecutor.h>
#include <stingray/toolkit/self_counter.h>
#include <stingray/toolkit/shared_ptr.h>
#include <stingray/threads/Thread.h>
#include <stingray/log/Logger.h>


namespace stingray
{

	namespace Detail
	{
		struct ISignalConnection : public self_counter<ISignalConnection>
		{
			struct VTable
			{
				typedef void ThisFunc(ISignalConnection *self);
				ThisFunc *Dtor, *Disconnect;

				VTable(ThisFunc *dtor, ThisFunc *disconnect) : Dtor(dtor), Disconnect(disconnect) { }
			};

			typedef VTable GetVTableFunc();
			GetVTableFunc	*_getVTable;

			ISignalConnection(): _getVTable(0) {}
			~ISignalConnection() { _getVTable().Dtor(this); }
		};
		TOOLKIT_DECLARE_SELF_COUNT_PTR(ISignalConnection);

		inline void DefaultSignalExceptionHandler(const std::exception& ex)
		{ Logger::Error() << "Uncaught  exception in signal handler: " << diagnostic_information(ex); }

		template<typename FuncType>
		struct FuncTypeWithDeathControl
		{
		private:
			FuncType					_func;
			TaskLifeToken				_token;

		public:
			FuncTypeWithDeathControl(const FuncType& func)
				: _func(func)
			{ }
			FuncTypeWithDeathControl(const FuncType& func, const TaskLifeToken& token)
				: _func(func), _token(token)
			{ }
			const FuncType& Func() 	{ return _func; }
			TaskLifeToken& Token()	{ return _token; }
		};

		template<typename Handlers, typename FuncType>
		class ThreadedConnection : public ISignalConnection
		{
		public:
			typedef std::pair<Handlers, Mutex>			HandlersType;
			typedef shared_ptr<HandlersType>			HandlersPtr;
			typedef weak_ptr<HandlersType>				HandlersWeakPtr;
			typedef typename ISignalConnection::VTable	VTable;

		private:
			HandlersWeakPtr							_handlers;
			typedef typename Handlers::iterator		IteratorType;
			IteratorType							_it;
			TaskLifeToken							_token;

		public:
			FORCE_INLINE ThreadedConnection(const HandlersWeakPtr& handlers, typename Handlers::iterator it, const TaskLifeToken& token)
				: _handlers(handlers), _it(it), _token(token)
			{ _getVTable = &GetVTable; }

			static VTable GetVTable()
			{ return VTable(&_Dtor, &_Disconnect); }

			static void _Dtor(ISignalConnection *self)
			{ static_cast<ThreadedConnection *>(self)->Dtor(); }

			static void _Disconnect(ISignalConnection *self)
			{ static_cast<ThreadedConnection *>(self)->Disconnect(); }

			void Dtor()
			{
				_handlers.~HandlersWeakPtr();
				_it.~IteratorType();
			}

			void Disconnect()
			{
				HandlersPtr handlers_l = _handlers.lock();
				if (!handlers_l)
					return;
				{
					MutexLock l(handlers_l->second);
					handlers_l->first.erase(_it);
				}
				_token.Release();
			}
		};

	} //namespace Detail

	class signal_connection
	{
	private:
		Detail::ISignalConnectionSelfCountPtr	_impl;

	public:
		FORCE_INLINE signal_connection()
		{ }

		FORCE_INLINE explicit signal_connection(const Detail::ISignalConnectionSelfCountPtr& impl)
			: _impl(impl)
		{ }

		inline bool connected() const { return _impl; }

		inline void disconnect()
		{
			if (!_impl)
				return;

			_impl->_getVTable().Disconnect(_impl.get());
			_impl.reset();
		}
	};

	class signal_connection_holder
	{
		signal_connection _connection;
	public:
		FORCE_INLINE signal_connection_holder() {}
		FORCE_INLINE signal_connection_holder(const signal_connection& connection) : _connection(connection) {}
		FORCE_INLINE signal_connection_holder(const signal_connection_holder &) : _connection() {}
		FORCE_INLINE ~signal_connection_holder() { _connection.disconnect(); }
		FORCE_INLINE bool connected() const { return _connection.connected(); }
		FORCE_INLINE void disconnect() { _connection.disconnect(); }

		FORCE_INLINE signal_connection_holder& operator=(const signal_connection &connection)
		{
			_connection.disconnect();
			_connection = connection;
			return *this;
		}
	};

	class signal_connection_autorelease_pool
	{
		typedef std::vector<signal_connection> signal_connection_list;
		signal_connection_list _connections;
		TOOLKIT_NONCOPYABLE(signal_connection_autorelease_pool);

	public:
		signal_connection_autorelease_pool()			{}
		inline ~signal_connection_autorelease_pool()	{ release(); }

		inline void add(const signal_connection & conn) { _connections.push_back(conn); }
		inline signal_connection_autorelease_pool& operator+= (const signal_connection& conn) { add(conn); return *this; }

		inline bool empty() const { return _connections.empty(); }

		void release()
		{
			std::for_each(_connections.begin(), _connections.end(),
				std::mem_fun_ref(&signal_connection::disconnect));
			_connections.clear();
		}
	};

}

#endif
