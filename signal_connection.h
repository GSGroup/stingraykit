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

	/**
	 * @addtogroup toolkit_functions
	 * @{
	 */

	namespace Detail
	{
		struct ISignalConnection : public self_counter<ISignalConnection>
		{
			virtual ~ISignalConnection() { }
			virtual void Disconnect() = 0;
		};
		TOOLKIT_DECLARE_SELF_COUNT_PTR(ISignalConnection);
	} //namespace Detail


	class signal_connection
	{
	private:
		Detail::ISignalConnectionSelfCountPtr	_impl;

	public:
		signal_connection()
		{ }

		explicit signal_connection(const Detail::ISignalConnectionSelfCountPtr& impl)
			: _impl(impl)
		{ }

		bool connected() const { return _impl; }

		void disconnect()
		{
			if (!_impl)
				return;

			_impl->Disconnect();
			_impl.reset();
		}
	};


	class signal_connection_holder
	{
		TOOLKIT_NONCOPYABLE(signal_connection_holder);

	private:
		signal_connection _connection;

	public:
		signal_connection_holder() {}
		signal_connection_holder(const signal_connection& connection) : _connection(connection) {}
		~signal_connection_holder() { _connection.disconnect(); }
		bool connected() const { return _connection.connected(); }
		void disconnect() { _connection.disconnect(); }

		signal_connection_holder& operator=(const signal_connection &connection)
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
		~signal_connection_pool()	{ release(); }

		void add(const signal_connection & conn)
		{
			MutexLock l(_lock);
			_connections.push_back(conn);
		}

		bool empty() const
		{
			MutexLock l(_lock);
			return _connections.empty();
		}

		void release()
		{
			signal_connection_list connections;
			{
				MutexLock l(_lock);
				connections.swap(_connections);
			}
			std::for_each(connections.rbegin(), connections.rend(),
				std::mem_fun_ref(&signal_connection::disconnect));
		}

		signal_connection_pool& operator+= (const signal_connection& conn) { add(conn); return *this; }
	};

	template < typename Key, typename Compare = std::less<Key> >
	class signal_connection_map
	{
		TOOLKIT_NONCOPYABLE(signal_connection_map);

		typedef std::multimap<Key, signal_connection, Compare> ConnectionMap;

		class BracketsOperatorProxy
		{
		private:
			Mutex&				_mutex;
			Key					_key;
			ConnectionMap*		_connections;

		public:
			BracketsOperatorProxy(Mutex& mutex, const Key& key, ConnectionMap& connections) :
				_mutex(mutex), _key(key), _connections(&connections)
			{ }

			BracketsOperatorProxy& operator+= (const signal_connection& connection)
			{
				MutexLock l(_mutex);
				_connections->insert(std::make_pair(_key, connection));
				return *this;
			}
		};

	private:
		Mutex			_mutex;
		ConnectionMap	_connections;

	public:
		signal_connection_map() { }
		~signal_connection_map()
		{ release_all(); }

		BracketsOperatorProxy operator[] (const Key& key) { return BracketsOperatorProxy(_mutex, key, _connections); }

		void release(const Key& key)
		{
			std::vector<signal_connection> connections;
			{
				MutexLock l(_mutex);
				typename ConnectionMap::iterator lower = _connections.lower_bound(key);
				typename ConnectionMap::iterator upper = _connections.upper_bound(key);
				std::copy(values_iterator(lower), values_iterator(upper), std::back_inserter(connections));
				_connections.erase(lower, upper);
			}
			std::for_each(connections.begin(), connections.end(), std::mem_fun_ref(&signal_connection::disconnect));
		}

		void release_all()
		{
			ConnectionMap connections;
			{
				MutexLock l(_mutex);
				_connections.swap(connections);
			}
			std::for_each(values_iterator(connections.begin()), values_iterator(connections.end()),
				std::mem_fun_ref(&signal_connection::disconnect));
		}
	};

	/** @} */

}

#endif
