// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/UUID.h>
#include <stingraykit/string/Hex.h>

#include <stdlib.h>
#include <algorithm>

namespace stingray
{

	const std::string UUID::Format = "XXXX-XX-XX-XX-XXXXXX";


	bool UUID::operator<(const UUID& other) const
	{
		return std::lexicographical_compare(_data.begin(), _data.end(), other._data.begin(), other._data.end());
	}


	UUID UUID::FromString(const std::string& str)
	{
		STINGRAYKIT_CHECK(str.size() == static_cast<std::string::size_type>(2 * std::count(Format.begin(), Format.end(), 'X') + std::count(Format.begin(), Format.end(), '-')), "Invalid format");

		UUID result;

		DataType::iterator data_it = result._data.begin();
		for (std::string::const_iterator format_it = Format.begin(), str_it = str.begin(); format_it != Format.end(); ++format_it)
		{
			if (*format_it == 'X')
				*data_it++ = FromHex<u8>(std::string((str_it++).base(), 2 * sizeof(u8)));
			else
				STINGRAYKIT_CHECK(*str_it == '-', "Expected '-'");
			++str_it;
		}

		return result;
	}


	std::string UUID::ToString() const
	{
		std::string result;

		DataType::const_iterator data_it = _data.begin();
		for (std::string::const_iterator format_it = Format.begin(); format_it != Format.end(); ++format_it)
			if (*format_it == 'X')
				result += ToHex(*data_it++, 2 * sizeof(u8));
			else
				result += *format_it;

		return result;
	}


	void UUID::GenerateImpl(UUID& uuid)
	{
		for (DataType::iterator it = uuid._data.begin(); it != uuid._data.end(); ++it)
			*it = rand();
	}

}
