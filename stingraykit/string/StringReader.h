#ifndef STINGRAYKIT_STRING_STRINGREADER_H
#define STINGRAYKIT_STRING_STRINGREADER_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/exception.h>

namespace stingray
{

	class StringReader
	{
	private:
		std::string			_text;
		size_t				_pos;

	public:
		StringReader(const std::string& text)
			:	_text(text),
				_pos(0)
		{ }

		std::string ReadLine()
		{
			size_t newPos = _pos;
			size_t delimeterSize = 0;

			while (newPos < _text.size())
			{
				const char ch = _text[newPos];

				if (ch == '\n')
				{
					delimeterSize = 1;
					break;
				}

				if (ch == '\r')
				{
					if ((newPos + 1) < _text.size() && _text[newPos + 1] == '\n')
						delimeterSize = 2;
					else
						delimeterSize = 1;

					break;
				}

				++newPos;
			}

			const std::string result = _text.substr(_pos, newPos - _pos);
			_pos = newPos + delimeterSize;

			return result;
		}

		std::string::value_type Peek()
		{
			STINGRAYKIT_CHECK(!IsEndOfString(), InvalidOperationException("EOF"));
			return _text[_pos];
		}

		bool IsEndOfString() const { return _pos == _text.size(); }
	};

}

#endif
