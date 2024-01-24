#ifndef STINGRAYKIT_IO_IINPUTBYTESTREAM_H
#define STINGRAYKIT_IO_IINPUTBYTESTREAM_H

// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
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

	struct IInputByteStream
	{
		virtual ~IInputByteStream() { }

		virtual u64 Read(ByteData data, const ICancellationToken& token = DummyCancellationToken()) = 0;
	};
	STINGRAYKIT_DECLARE_PTR(IInputByteStream);


	template < typename StreamType >
	size_t ReadAll(StreamType&& stream, ByteData data, const ICancellationToken& token = DummyCancellationToken())
	{
		size_t total = 0;
		optional<size_t> readed;
		while (token && (total < data.size()) && (!readed || *readed != 0))
		{
			readed = (size_t)stream.Read(ByteData(data, total), token);
			total += *readed;
		}

		return total;
	}


	template < typename StreamType >
	void CheckedReadAll(StreamType&& stream, ByteData data, const ICancellationToken& token = DummyCancellationToken())
	{
		const size_t readed = ReadAll(std::forward<StreamType>(stream), data, token);
		STINGRAYKIT_CHECK(readed == data.size(), InputOutputException(StringBuilder() % "Readed only " % readed % " of " % data.size()));
	}


	template < typename StreamType >
	ConstByteArray ReadToEnd(StreamType&& stream, size_t initialSize, const ICancellationToken& token = DummyCancellationToken())
	{
		STINGRAYKIT_CHECK(initialSize, ArgumentException("initialSize"));

		size_t offset = 0;
		ByteArray buffer(initialSize);

		while (token)
		{
			const ByteData toRead(buffer, offset);
			const size_t readed = ReadAll(stream, toRead, token);

			offset += readed;
			if (readed != toRead.size())
				break;

			buffer.RequireSize(buffer.size() * 2);
		}

		return ConstByteArray(buffer, 0, offset);
	}

}

#endif
