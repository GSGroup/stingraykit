// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/UUID.h>

#include <stingraykit/string/Hex.h>
#include <stingraykit/string/ToString.h>

#include <algorithm>

namespace stingray
{

	namespace
	{

		const char FormatStr[] = "XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX";

		const string_view Format(FormatStr);

		constexpr u8 Version = 0x40;

		constexpr u8 VersionMask = 0x0f;

		constexpr u8 Variant = 0x80;

		constexpr u8 VariantMask = 0x3f;

		void SetVersionAndVariant(UUID::DataType& data)
		{
			data[6] &= VersionMask;
			data[6] |= Version;

			data[8] &= VariantMask;
			data[8] |= Variant;
		}

	}


	UUID::UUID(ConstByteData data, bool setVersionAndVariant)
	{
		STINGRAYKIT_CHECK(data.size() == DataType::Size, ArgumentException("data.size()", data.size()));

		std::copy(data.begin(), data.end(), _data.begin());
		if (setVersionAndVariant)
			SetVersionAndVariant(_data);
	}


	UUID UUID::Generate()
	{
		UUID uuid;
		std::generate(uuid._data.begin(), uuid._data.end(), std::rand);
		SetVersionAndVariant(uuid._data);
		return uuid;
	}


	UUID UUID::FromString(string_view str)
	{
		STINGRAYKIT_CHECK(str.length() == Format.length(), FormatException("invalid length!"));

		UUID result;

		DataType::iterator resIt = result._data.begin();
		string_view::const_pointer strIt = str.data();
		for (string_view::const_iterator fmtIt = Format.begin(), end = Format.end(); fmtIt != end; ++fmtIt, ++strIt)
		{
			if (*fmtIt == 'X')
			{
				*resIt++ = FromHex<u8>(string_view(strIt++, 2));
				++fmtIt;
			}
			else
				STINGRAYKIT_CHECK(*strIt == *fmtIt, FormatException(StringBuilder() % "expected '" % *fmtIt % "'!"));
		}

		return result;
	}


	std::string UUID::ToString() const
	{
		std::string result;
		result.reserve(Format.length());

		DataType::const_iterator dataIt = _data.begin();
		for (string_view::const_iterator fmtIt = Format.begin(), end = Format.end(); fmtIt != end; ++fmtIt)
			if (*fmtIt == 'X')
			{
				result += ToHex(*dataIt++, 2);
				++fmtIt;
			}
			else
				result += *fmtIt;

		return result;
	}


	string_view UUID::GetRepresentation()
	{ return Format; }

}
