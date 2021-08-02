#ifndef STINGRAYKIT_UUID_H
#define STINGRAYKIT_UUID_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/ByteData.h>
#include <stingraykit/string/string_view.h>
#include <stingraykit/toolkit.h>

namespace stingray
{

	class UUID
	{
	public:
		using DataType = array<u8, 16>;

	private:
		DataType _data;

	public:
		UUID() { }
		explicit UUID(ConstByteData data, bool setVersionAndVariant = false);

		bool operator< (const UUID& other) const
		{ return std::lexicographical_compare(_data.begin(), _data.end(), other._data.begin(), other._data.end()); }
		STINGRAYKIT_GENERATE_COMPARISON_OPERATORS_FROM_LESS(UUID);

		static UUID Generate();
		static UUID FromString(string_view str);
		static string_view GetRepresentation();

		std::string ToString() const;
	};

}

#endif
