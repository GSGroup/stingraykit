#ifndef STINGRAYKIT_IO_IBYTESTREAM_H
#define STINGRAYKIT_IO_IBYTESTREAM_H

// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/io/IInputByteStream.h>
#include <stingraykit/io/IOutputByteStream.h>
#include <stingraykit/io/SeekMode.h>
#include <stingraykit/ICreator.h>

namespace stingray
{

	struct IByteStream : public virtual IInputByteStream, public virtual IOutputByteStream
	{
		virtual void Seek(s64 offset, SeekMode mode = SeekMode::Begin) = 0;
		virtual u64 Tell() const = 0;
	};
	STINGRAYKIT_DECLARE_PTR(IByteStream);
	STINGRAYKIT_DECLARE_CREATOR(IByteStream);


	template < typename StreamType >
	inline optional<std::string> ReadLine(StreamType& stream, const ICancellationToken& token = DummyCancellationToken())
	{
		optional<string_ostream> result;

		while (token)
		{
			u8 byte;
			if (stream.Read(ByteData(&byte, sizeof(byte)), token) == 0)
				break;

			if (!result)
				result.emplace();

			if (byte == '\n')
				break;

			if (byte == '\r')
			{
				if (stream.Read(ByteData(&byte, sizeof(byte)), token) == 0 || byte == '\n')
					break;

				stream.Seek(-1, SeekMode::Current);
				break;
			}

			(*result) << static_cast<string_ostream::value_type>(byte);
		}

		STINGRAYKIT_CHECK_CANCELLATION(token);
		return result ? make_optional_value(result->str()) : null;
	}

}

#endif
