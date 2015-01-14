#ifndef STINGRAYKIT_ITOKEN_H
#define STINGRAYKIT_ITOKEN_H

#include <map>

#include <stingray/toolkit/collection/iterators.h>
#include <stingray/toolkit/shared_ptr.h>
#include <stingray/toolkit/thread/Thread.h>


namespace stingray
{

	struct IToken
	{
		virtual ~IToken() { }
	};
	STINGRAYKIT_DECLARE_PTR(IToken);


	class Token : public safe_bool<Token>
	{
	private:
		ITokenPtr	_token;

	public:
		Token()
		{ }

		template<typename T>
		Token(const T& token) : _token(token)
		{ }

		Token& operator = (const Token& token)		{ Set(token); return *this; }

		void Set(const Token& token)				{ _token = token._token; }
		void Reset()								{ Set(null); }

		bool IsInitialized() const					{ return _token; }
		bool boolean_test() const					{ return IsInitialized(); }
	};


	class TokenReturnProxy
	{
		STINGRAYKIT_NONASSIGNABLE(TokenReturnProxy);

		class Impl;
		STINGRAYKIT_DECLARE_PTR(Impl);

	private:
		ImplPtr				_impl;

	public:
		TokenReturnProxy(const Token& token);
		TokenReturnProxy(const TokenReturnProxy& other);
		~TokenReturnProxy();

		operator Token() const;
	};


	class TokenHolder
	{
	private:
		Mutex		_mutex;
		Token		_token;

	public:
		TokenHolder()
		{ }

		TokenHolder(const Token& token) : _token(token)
		{ }

		void Set(const Token& token)
		{
			Token local_copy = token;
			{
				MutexLock l(_mutex);
				std::swap(local_copy, _token);
			}
		}

		void Reset()
		{ Set(null); }

		TokenHolder& operator = (const Token& token)
		{
			Set(token);
			return *this;
		}
	};


	class TokenPool
	{
		STINGRAYKIT_NONCOPYABLE(TokenPool);

		typedef std::vector<Token> Tokens;

	private:
		Tokens		_tokens;
		Mutex		_mutex;

	public:
		TokenPool()									{ }
		~TokenPool()								{ Release(); }

		bool Empty() const							{ MutexLock l(_mutex); return _tokens.empty(); }

		void Add(const Token& token)				{ MutexLock l(_mutex); _tokens.push_back(token); }
		TokenPool& operator+= (const Token& token)	{ Add(token); return *this; }

		void Release()
		{
			Tokens tokens;
			{
				MutexLock l(_mutex);
				tokens.swap(_tokens);
			}
			while (!tokens.empty())
				tokens.pop_back();
		}
	};


	template < typename Key, typename Compare = std::less<Key> >
	class TokenMap
	{
		STINGRAYKIT_NONCOPYABLE(TokenMap);

		typedef std::multimap<Key, Token, Compare> Tokens;

		class BracketsOperatorProxy
		{
		private:
			Mutex&		_mutex;
			Key			_key;
			Tokens*		_tokens;

		public:
			BracketsOperatorProxy(Mutex& mutex, const Key& key, Tokens& tokens) : _mutex(mutex), _key(key), _tokens(&tokens) { }

			BracketsOperatorProxy& operator+= (const Token& token)
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
			std::vector<Token> tokens;
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
