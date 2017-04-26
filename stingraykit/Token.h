#ifndef STINGRAYKIT_TOKEN_H
#define STINGRAYKIT_TOKEN_H

// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <map>

#include <stingraykit/shared_ptr.h>
#include <stingraykit/thread/Thread.h>


namespace stingray
{

	struct IToken : public self_counter<IToken>
	{
		virtual ~IToken() { }
	};


	class Token : public safe_bool<Token>
	{
	private:
		self_count_ptr<IToken>	_token;

	public:
		Token()
		{ }

		Token(const NullPtrType& token)
		{ }

		template<typename T>
		Token(const self_count_ptr<T>& token) : _token(token)
		{ }

		Token& operator = (const Token& token)		{ Set(token); return *this; }

		void Set(const Token& token)				{ _token = token._token; }
		void Reset()								{ Set(null); }

		bool IsInitialized() const					{ return _token; }
		bool boolean_test() const					{ return IsInitialized(); }
	};


#define DETAIL_DECL_MAKE_TOKEN(N_, UserArg_) \
	template< typename T STINGRAYKIT_COMMA_IF(N_) STINGRAYKIT_REPEAT(N_, STINGRAYKIT_TEMPLATE_PARAM_DECL, Param) > \
	Token MakeToken(STINGRAYKIT_REPEAT(N_, STINGRAYKIT_FUNCTION_PARAM_DECL, Param)) \
	{ return self_count_ptr<IToken>(new T(STINGRAYKIT_REPEAT(N_, STINGRAYKIT_FUNCTION_PARAM_USAGE, STINGRAYKIT_EMPTY()))); }


	STINGRAYKIT_REPEAT_NESTING_2(10, DETAIL_DECL_MAKE_TOKEN, STINGRAYKIT_EMPTY())


#undef DETAIL_DECL_MAKE_TOKEN


	namespace Detail
	{
		template<typename T>
		struct AttachTokenCustomDeleter
		{
		private:
			shared_ptr<T> _ptr;
			Token         _token;

		public:
			AttachTokenCustomDeleter(const shared_ptr<T>& ptr, const Token& token) :
				_ptr(ptr), _token(token)
			{ }

			void operator() (T* ptr)
			{
				_token.Reset();
				_ptr.reset();
			}
		};
	}


	template<typename T>
	shared_ptr<T> AttachToken(const shared_ptr<T>& ptr, const Token& token)
	{ return shared_ptr<T>(ptr.get(), Detail::AttachTokenCustomDeleter<T>(ptr, token)); }


	template < typename T >
	class TokenAttacher
	{
	private:
		shared_ptr<T>	_ptr;

	public:
		explicit TokenAttacher(const shared_ptr<T>& ptr) : _ptr(ptr) { }

		TokenAttacher& operator % (const Token& token)
		{
			_ptr = AttachToken(_ptr, token);
			return *this;
		}

		operator shared_ptr<T> () const
		{ return _ptr; }
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


	class ThreadlessTokenPool
	{
		STINGRAYKIT_NONCOPYABLE(ThreadlessTokenPool);

		typedef std::vector<Token> Tokens;

	private:
		Tokens	_tokens;

	public:
		ThreadlessTokenPool()									{ }
		~ThreadlessTokenPool()									{ Release(); }

		bool Empty() const										{ return _tokens.empty(); }

		void Add(const Token& token)							{ _tokens.push_back(token); }
		ThreadlessTokenPool& operator+= (const Token& token)	{ Add(token); return *this; }

		void Release()
		{
			while (!Empty())
				_tokens.pop_back();
			Tokens().swap(_tokens);
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

		bool contains(const Key& key) const
		{
			MutexLock l(_mutex);
			return _tokens.find(key) != _tokens.end();
		}

		BracketsOperatorProxy operator[] (const Key& key) { return BracketsOperatorProxy(_mutex, key, _tokens); }

		void release(const Key& key)
		{
			std::vector<Token> tokens;
			{
				MutexLock l(_mutex);
				std::pair<typename Tokens::iterator, typename Tokens::iterator> range = _tokens.equal_range(key);
				for (typename Tokens::iterator it = range.first; it != range.second; ++it)
					tokens.push_back(it->second);
				_tokens.erase(range.first, range.second);
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
