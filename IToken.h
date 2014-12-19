#ifndef STINGRAY_TOOLKIT_ITOKEN_H
#define STINGRAY_TOOLKIT_ITOKEN_H

#include <map>

#include <stingray/toolkit/thread/Thread.h>
#include <stingray/toolkit/shared_ptr.h>


namespace stingray
{

	struct IToken
	{
		virtual ~IToken() { }
	};
	TOOLKIT_DECLARE_PTR(IToken);


	class TokenPool
	{
		TOOLKIT_NONCOPYABLE(TokenPool);

		typedef std::vector<ITokenPtr> Tokens;

	private:
		Tokens		_tokens;
		Mutex		_mutex;

	public:
		TokenPool() { }
		~TokenPool() { Release(); }

		void Add(const ITokenPtr& token)
		{
			MutexLock l(_mutex);
			_tokens.push_back(token);
		}

		bool Empty() const
		{
			MutexLock l(_mutex);
			return _tokens.empty();
		}

		void Release()
		{
			Tokens tokens;
			{
				MutexLock l(_mutex);
				tokens.swap(_tokens);
			}
		}

		TokenPool& operator+= (const ITokenPtr& token)
		{
			Add(token);
			return *this;
		}

	};


	template < typename Key, typename Compare = std::less<Key> >
	class TokenMap
	{
		TOOLKIT_NONCOPYABLE(TokenMap);

		typedef std::multimap<Key, ITokenPtr, Compare> Tokens;

		class BracketsOperatorProxy
		{
		private:
			Mutex&		_mutex;
			Key			_key;
			Tokens*		_tokens;

		public:
			BracketsOperatorProxy(Mutex& mutex, const Key& key, Tokens& tokens) : _mutex(mutex), _key(key), _tokens(&tokens) { }

			BracketsOperatorProxy& operator+= (const ITokenPtr& token)
			{
				MutexLock l(_mutex);
				_tokens->insert(std::make_pair(_key, token));
				return *this;
			}
		};

	private:
		Mutex		_mutex;
		Tokens		_tokens;

	public:
		TokenMap() { }
		~TokenMap() { release_all(); }

		BracketsOperatorProxy operator[] (const Key& key) { return BracketsOperatorProxy(_mutex, key, _tokens); }

		void release(const Key& key)
		{
			std::vector<ITokenPtr> tokens;
			{
				MutexLock l(_mutex);
				typename Tokens::iterator lower = _tokens.lower_bound(key);
				typename Tokens::iterator upper = _tokens.upper_bound(key);
				std::copy(values_iterator(lower), values_iterator(upper), std::back_inserter(tokens));
				_tokens.erase(lower, upper);
			}
		}

		void release_all()
		{
			Tokens tokens;
			{
				MutexLock l(_mutex);
				_tokens.swap(tokens);
			}
		}
	};

}


#endif
