#ifndef STINGRAYKIT_UUID_H
#define STINGRAYKIT_UUID_H

// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <stingraykit/collection/array.h>
#include <stingraykit/toolkit.h>


namespace stingray
{

	struct UUID
	{
	private:
		typedef array<u8, 16> DataType;
		DataType _data;

	public:
		bool operator< (const UUID& other) const;
		STINGRAYKIT_GENERATE_COMPARISON_OPERATORS_FROM_LESS(UUID);

		static UUID Generate()
		{
			UUID result;
			GenerateImpl(result);
			return result;
		}

		static UUID FromString(const std::string& str);

		std::string ToString() const;

	private:
		static void GenerateImpl(UUID& uuid);
	};

}

#endif
