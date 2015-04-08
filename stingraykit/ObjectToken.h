#ifndef STINGRAYKIT_OBJECTTOKEN_H
#define STINGRAYKIT_OBJECTTOKEN_H

// Copyright (c) 2011 - 2015, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <stingraykit/Token.h>


namespace stingray
{

	namespace Detail
	{
		template <typename T>
		struct ObjectToken : public virtual IToken
		{
		private:
			T	_object;

		public:
			ObjectToken(const T& object) : _object(object)
			{ }
		};
	}


	template <typename T>
	Token MakeObjectToken(const T& object)
	{ return MakeToken<Detail::ObjectToken<T> >(object); }

}

#endif
