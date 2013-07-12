#ifndef STINGRAY_TOOLKIT_SIGNAL_CONNECTION_H
#define STINGRAY_TOOLKIT_SIGNAL_CONNECTION_H

#include <algorithm>
#include <map>
#include <vector>

#include <stingray/log/Logger.h>
#include <stingray/threads/Thread.h>
#include <stingray/toolkit/iterators.h>
#include <stingray/toolkit/self_counter.h>
#include <stingray/toolkit/shared_ptr.h>
#include <stingray/toolkit/task_alive_token.h>


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

		struct FuncTypeWithDeathControl
		{
		private:
			function_storage		_func;
			FutureExecutionTester	_tester;

		public:
			FuncTypeWithDeathControl(const function_storage& func, const FutureExecutionTester& tester = null)
				: _func(func), _tester(tester)
			{ }
			const function_storage& Func() 	{ return _func; }
			FutureExecutionTester& Tester()	{ return _tester; }
		};

		template<typename Handlers>
		class ThreadedConnection : public ISignalConnection
		{
		public:
			typedef std::pair<Handlers, Mutex>			HandlersType;
			typedef shared_ptr<HandlersType>			HandlersPtr;
			typedef weak_ptr<HandlersType>				HandlersWeakPtr;
			typedef typename ISignalConnection::VTable	VTable;

		private:
			typedef typename Handlers::iterator		IteratorType;

			HandlersWeakPtr							_handlers;
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
				_token.~TaskLifeToken();
				_it.~IteratorType();
				_handlers.~HandlersWeakPtr();
			}

			void Disconnect()
			{
				HandlersPtr handlers_l = _handlers.lock();
				if (handlers_l)
				{
					MutexLock l(handlers_l->second);
					_handlers.reset();
					Handlers &handlers = handlers_l->first;
					if (_it != handlers.end())
					{
						handlers.erase(_it);
						_it = handlers.end();
					}
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
		TOOLKIT_NONCOPYABLE(signal_connection_holder);

	private:
		signal_connection _connection;

	public:
		FORCE_INLINE signal_connection_holder() {}
		FORCE_INLINE signal_connection_holder(const signal_connection& connection) : _connection(connection) {}
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

	class signal_connection_pool
	{
		TOOLKIT_NONCOPYABLE(signal_connection_pool);

		typedef std::vector<signal_connection> signal_connection_list;
		signal_connection_list _connections;

		Mutex _lock;

	public:
		signal_connection_pool()			{}
		inline ~signal_connection_pool()	{ release(); }

		inline void add(const signal_connection & conn)
		{
			MutexLock l(_lock);
			_connections.push_back(conn);
		}

		inline bool empty() const
		{
			MutexLock l(_lock);
			return _connections.empty();
		}

		void release()
		{
			MutexLock l(_lock);
			std::for_each(_connections.rbegin(), _connections.rend(),
				std::mem_fun_ref(&signal_connection::disconnect));
			_connections.clear();
		}

		inline signal_connection_pool& operator+= (const signal_connection& conn) { add(conn); return *this; }
	};

	template < typename Key, typename Compare = std::less<Key> >
	class signal_connection_map
	{
		TOOLKIT_NONCOPYABLE(signal_connection_map);

		typedef std::multimap<Key, signal_connection, Compare> ConnectionMap;

		class BracketsOperatorProxy
		{
		private:
			Key					_key;
			ConnectionMap*		_connections;

		public:
			BracketsOperatorProxy(const Key& key, ConnectionMap& connections)
				: _key(key), _connections(&connections)
			{ }

			BracketsOperatorProxy& operator+= (const signal_connection& connection)
			{
				_connections->insert(std::make_pair(_key, connection));
				return *this;
			}
		};

	private:
		ConnectionMap	_connections;

	public:
		signal_connection_map() { }
		~signal_connection_map()
		{ release_all(); }

		BracketsOperatorProxy operator[] (const Key& key) { return BracketsOperatorProxy(key, _connections); }

		void release(const Key& key)
		{
			typename ConnectionMap::iterator lower = _connections.lower_bound(key);
			typename ConnectionMap::iterator upper = _connections.upper_bound(key);
			std::for_each(values_iterator(lower), values_iterator(upper),
				std::mem_fun_ref(&signal_connection::disconnect));
			_connections.erase(lower, upper);
		}

		void release_all()
		{
			std::for_each(values_iterator(_connections.begin()), values_iterator(_connections.end()),
				std::mem_fun_ref(&signal_connection::disconnect));
			_connections.clear();
		}
	};

}

#endif
