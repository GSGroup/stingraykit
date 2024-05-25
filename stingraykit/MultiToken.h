#ifndef STINGRAYKIT_MULTITOKEN_H
#define STINGRAYKIT_MULTITOKEN_H

// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/ObjectToken.h>
#include <stingraykit/unique_ptr.h>

#include <deque>

namespace stingray
{

	template < bool IsForwardDestructionOrder >
	class MultiTokenBuilderBase
	{
		class MultiToken
		{
			STINGRAYKIT_NONCOPYABLE(MultiToken);

		private:
			using Tokens = std::deque<Token>;

		private:
			Tokens		_tokens;

		public:
			MultiToken() { }

			~MultiToken()
			{
				while (!_tokens.empty())
				{
					if (IsForwardDestructionOrder)
						_tokens.pop_front();
					else
						_tokens.pop_back();
				}
			}

			void Add(const Token& token)
			{ _tokens.push_back(token); }

			void Add(Token&& token)
			{ _tokens.push_back(std::move(token)); }
		};
		STINGRAYKIT_DECLARE_UNIQ_PTR(MultiToken);

	private:
		MultiTokenUniqPtr		_token;

	public:
		MultiTokenBuilderBase()
			:	_token(make_unique_ptr<MultiToken>())
		{ }

		MultiTokenBuilderBase& operator % (const Token& token)
		{ STINGRAYKIT_REQUIRE_NOT_NULL(_token)->Add(token); return *this; }

		MultiTokenBuilderBase& operator % (Token&& token)
		{ STINGRAYKIT_REQUIRE_NOT_NULL(_token)->Add(std::move(token)); return *this; }

		operator Token ()
		{ return MakeObjectToken(MultiTokenUniqPtr(std::move(STINGRAYKIT_REQUIRE_NOT_NULL(_token)))); }
	};


	using ForwardMultiTokenBuilder = MultiTokenBuilderBase<true>;
	using ReverseMultiTokenBuilder = MultiTokenBuilderBase<false>;

}

#endif
