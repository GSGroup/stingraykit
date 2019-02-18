// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/UUID.h>

#include <stingraykit/string/Hex.h>
#include <stingraykit/string/ToString.h>

#include <algorithm>
#include <cstdlib>

namespace stingray
{

	namespace
	{

		const char FormatStr[] = "XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX";

		const string_view Format(FormatStr);

	}


	UUID UUID::FromString(string_view str)
	{
		STINGRAYKIT_CHECK(str.length() == Format.length(), FormatException("invalid length!"));

		UUID result;

		DataType::iterator res_it = result._data.begin();
		string_view::const_pointer str_it = str.data();
		for (string_view::const_iterator fmt_it = Format.begin(), end = Format.end(); fmt_it != end; ++fmt_it, ++str_it)
		{
			if (*fmt_it == 'X')
			{
				*res_it++ = FromHex<u8>(string_view(str_it++, 2));
				++fmt_it;
			}
			else
				STINGRAYKIT_CHECK(*str_it == *fmt_it, FormatException(StringBuilder() % "expected '" % *fmt_it % "'!"));
		}

		return result;
	}


	std::string UUID::ToString() const
	{
		std::string result;
		result.reserve(Format.length());

		DataType::const_iterator data_it = _data.begin();
		for (string_view::const_iterator fmt_it = Format.begin(), end = Format.end(); fmt_it != end; ++fmt_it)
			if (*fmt_it == 'X')
			{
				result += ToHex(*data_it++, 2);
				++fmt_it;
			}
			else
				result += *fmt_it;

		return result;
	}


	string_view UUID::GetRepresentation()
	{ return Format; }

}
