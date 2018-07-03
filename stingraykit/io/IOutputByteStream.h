#ifndef STINGRAYKIT_IO_IOUTPUTBYTESTREAM_H
#define STINGRAYKIT_IO_IOUTPUTBYTESTREAM_H

// Copyright (c) 2011 - 2018, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/ByteData.h>
#include <stingraykit/string/ToString.h>
#include <stingraykit/thread/DummyCancellationToken.h>

namespace stingray
{

	struct IOutputByteStream
	{
		virtual ~IOutputByteStream() { }

		virtual u64 Write(ConstByteData data, const ICancellationToken& token = DummyCancellationToken()) = 0;
	};
	STINGRAYKIT_DECLARE_PTR(IOutputByteStream);


	template < typename StreamType >
	inline size_t WriteAll(StreamType& stream, ConstByteData data, const ICancellationToken& token = DummyCancellationToken())
	{
		size_t total = 0;
		size_t written = 0;
		do {
			written = (size_t)stream.Write(ConstByteData(data, total), token);
			total += written;
		} while (token && (total < data.size()) && written != 0);

		return total;
	}


	template < typename StreamType >
	inline void CheckedWriteAll(StreamType& stream, ConstByteData data, const ICancellationToken& token = DummyCancellationToken())
	{
		const size_t written = WriteAll(stream, data, token);
		STINGRAYKIT_CHECK(written == data.size(), InputOutputException(StringBuilder() % "Written only " % written % " of " % data.size()));
	}

}

#endif
