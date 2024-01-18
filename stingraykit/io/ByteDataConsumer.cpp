// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/io/ByteDataConsumer.h>

#include <string.h>

namespace stingray
{

	ByteDataConsumer::ByteDataConsumer(ByteData destination)
		:	_destination(destination),
			_eod(false)
	{ }


	size_t ByteDataConsumer::Process(ConstByteData data, const ICancellationToken&)
	{
		const size_t size = std::min(data.size(), _destination.size());
		memcpy(_destination.data(), data.data(), size);
		_destination = ByteData(_destination, size);

		return data.size();
	}

}
