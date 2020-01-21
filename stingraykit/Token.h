#ifndef STINGRAYKIT_TOKEN_H
#define STINGRAYKIT_TOKEN_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/iterators.h>
#include <stingraykit/thread/Thread.h>

namespace stingray
{

	struct IToken : public self_counter<IToken>
	{
		virtual ~IToken() { }
	};


	class Token
	{
		STINGRAYKIT_DEFAULTCOPYABLE(Token);
		STINGRAYKIT_DEFAULTMOVABLE(Token);

	private:
		self_count_ptr<IToken>	_token;

	public:
		Token()
		{ }

		Token(const NullPtrType&)
		{ }

		template < typename T >
		Token(const self_count_ptr<T>& token)
			: _token(token)
		{ }

		template < typename T >
		Token(self_count_ptr<T>&& token)
			: _token(std::move(token))
		{ }

		void Set(const Token& token)				{ _token = token._token; }
		void Set(Token&& token)						{ _token = std::move(token._token); }
		void Reset()								{ Set(null); }

		bool IsInitialized() const					{ return _token.is_initialized(); }
		explicit operator bool () const				{ return IsInitialized(); }
	};


	template < typename T, typename... Us >
	Token MakeToken(Us&&... args)
	{ return self_count_ptr<IToken>(new T(std::forward<Us>(args)...)); }


	template < >
	struct IsNullable<Token> : public TrueType { };


	namespace Detail
	{
		template < typename T >
		struct AttachTokenCustomDeleter
		{
		private:
			shared_ptr<T> _ptr;
			Token         _token;

		public:
			AttachTokenCustomDeleter(const shared_ptr<T>& ptr, const Token& token)
				: _ptr(ptr), _token(token)
			{ }

			AttachTokenCustomDeleter(const shared_ptr<T>& ptr, Token&& token)
				: _ptr(ptr), _token(std::move(token))
			{ }

			void operator () (T*)
			{
				_token.Reset();
				_ptr.reset();
			}
		};
	}


	template < typename T >
	shared_ptr<T> AttachToken(const shared_ptr<T>& ptr, const Token& token)
	{ return shared_ptr<T>(ptr.get(), Detail::AttachTokenCustomDeleter<T>(ptr, token)); }


	template < typename T >
	shared_ptr<T> AttachToken(const shared_ptr<T>& ptr, Token&& token)
	{ return shared_ptr<T>(ptr.get(), Detail::AttachTokenCustomDeleter<T>(ptr, std::move(token))); }


	template < typename T >
	class TokenAttacher
	{
	private:
		shared_ptr<T>	_ptr;

	public:
		explicit TokenAttacher(const shared_ptr<T>& ptr)
			: _ptr(ptr)
		{ }

		TokenAttacher& operator % (const Token& token)
		{ _ptr = AttachToken(_ptr, token); return *this; }

		TokenAttacher& operator % (Token&& token)
		{ _ptr = AttachToken(_ptr, std::move(token)); return *this; }

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

		TokenHolder(const Token& token)
			: _token(token)
		{ }

		TokenHolder(Token&& token)
			: _token(std::move(token))
		{ }

		void Set(const Token& token_)
		{
			Token token(token_);

			MutexLock l(_mutex);
			std::swap(token, _token);
		}

		void Set(Token&& token_)
		{
			Token token(std::move(token_));

			MutexLock l(_mutex);
			std::swap(token, _token);
		}

		void Reset()
		{ Set(null); }

		TokenHolder& operator = (const Token& token)
		{ Set(token); return *this; }

		TokenHolder& operator = (Token&& token)
		{ Set(std::move(token)); return *this; }
	};


	class ThreadlessTokenPool
	{
		STINGRAYKIT_NONCOPYABLE(ThreadlessTokenPool);

	private:
		typedef std::vector<Token> Tokens;

	private:
		Tokens	_tokens;

	public:
		ThreadlessTokenPool()
		{ }

		bool Empty() const										{ return _tokens.empty(); }

		void Add(const Token& token)							{ _tokens.push_back(token); }
		void Add(Token&& token)									{ _tokens.push_back(std::move(token)); }

		ThreadlessTokenPool& operator += (const Token& token)	{ Add(token); return *this; }
		ThreadlessTokenPool& operator += (Token&& token)		{ Add(std::move(token)); return *this; }

		void Release()											{ Tokens().swap(_tokens); }
	};


	class TokenPool
	{
		STINGRAYKIT_NONCOPYABLE(TokenPool);

	private:
		typedef std::vector<Token> Tokens;

	private:
		Mutex		_mutex;
		Tokens		_tokens;

	public:
		TokenPool()									{ }

		bool Empty() const							{ MutexLock l(_mutex); return _tokens.empty(); }

		void Add(const Token& token)				{ MutexLock l(_mutex); _tokens.push_back(token); }
		void Add(Token&& token)						{ MutexLock l(_mutex); _tokens.push_back(std::move(token)); }

		TokenPool& operator += (const Token& token)	{ Add(token); return *this; }
		TokenPool& operator += (Token&& token)		{ Add(std::move(token)); return *this; }

		void Release()
		{
			Tokens tokens;

			MutexLock l(_mutex);
			tokens.swap(_tokens);
		}
	};


	template < typename Key, typename Compare = comparers::Less >
	class ThreadlessTokenMap
	{
		STINGRAYKIT_NONCOPYABLE(ThreadlessTokenMap);

	private:
		typedef std::multimap<Key, Token, Compare> Tokens;

		class BracketsOperatorProxy
		{
		private:
			Tokens&		_tokens;
			Key			_key;

		public:
			BracketsOperatorProxy(Tokens& tokens, const Key& key)
				: _tokens(tokens), _key(key)
			{ }

			BracketsOperatorProxy(Tokens& tokens, Key&& key)
				: _tokens(tokens), _key(std::move(key))
			{ }

			BracketsOperatorProxy& operator += (const Token& token)
			{ _tokens.emplace(_key, token); return *this; }

			BracketsOperatorProxy& operator += (Token&& token)
			{ _tokens.emplace(_key, std::move(token)); return *this; }
		};

	private:
		Tokens		_tokens;

	public:
		ThreadlessTokenMap()
		{ }

		bool contains(const Key& key) const
		{ return _tokens.find(key) != _tokens.end(); }

		BracketsOperatorProxy operator [] (const Key& key)
		{ return BracketsOperatorProxy(_tokens, key); }

		BracketsOperatorProxy operator [] (Key&& key)
		{ return BracketsOperatorProxy(_tokens, std::move(key)); }

		void release(const Key& key)
		{ _tokens.erase(key); }

		void release_all()
		{ _tokens.clear(); }
	};


	template < typename Key, typename Compare = comparers::Less >
	class TokenMap
	{
		STINGRAYKIT_NONCOPYABLE(TokenMap);

	private:
		typedef std::multimap<Key, Token, Compare> Tokens;

		class BracketsOperatorProxy
		{
		private:
			Mutex&		_mutex;
			Tokens&		_tokens;
			Key			_key;

		public:
			BracketsOperatorProxy(Mutex& mutex, Tokens& tokens, const Key& key)
				: _mutex(mutex), _tokens(tokens), _key(key)
			{ }

			BracketsOperatorProxy(Mutex& mutex, Tokens& tokens, Key&& key)
				: _mutex(mutex), _tokens(tokens), _key(std::move(key))
			{ }

			BracketsOperatorProxy& operator += (const Token& token)
			{
				MutexLock l(_mutex);
				_tokens.emplace(_key, token);
				return *this;
			}

			BracketsOperatorProxy& operator += (Token&& token)
			{
				MutexLock l(_mutex);
				_tokens.emplace(_key, std::move(token));
				return *this;
			}
		};

	private:
		Mutex		_mutex;
		Tokens		_tokens;

	public:
		TokenMap()
		{ }

		bool contains(const Key& key) const
		{ MutexLock l(_mutex); return _tokens.find(key) != _tokens.end(); }

		BracketsOperatorProxy operator [] (const Key& key)
		{ return BracketsOperatorProxy(_mutex, _tokens, key); }

		BracketsOperatorProxy operator [] (Key&& key)
		{ return BracketsOperatorProxy(_mutex, _tokens, std::move(key)); }

		void release(const Key& key)
		{
			std::vector<Token> tokens;

			MutexLock l(_mutex);
			const std::pair<typename Tokens::iterator, typename Tokens::iterator> range = _tokens.equal_range(key);
			tokens.assign(values_iterator(range.first), values_iterator(range.second));
			_tokens.erase(range.first, range.second);
		}

		void release_all()
		{
			Tokens tokens;

			MutexLock l(_mutex);
			_tokens.swap(tokens);
		}
	};

}

#endif
