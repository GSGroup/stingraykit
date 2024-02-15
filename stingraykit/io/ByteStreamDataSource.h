#ifndef STINGRAYKIT_IO_BYTESTREAMDATASOURCE_H
#define STINGRAYKIT_IO_BYTESTREAMDATASOURCE_H

// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/io/IByteStream.h>
#include <stingraykit/io/IDataSource.h>

namespace stingray
{

	class ByteStreamDataSource final : public virtual IDataSource
	{
		static const size_t	DefaultBufferSize = 128 * 1024;

	private:
		IByteStreamPtr		_stream;

		ByteArray			_buffer;

	public:
		explicit ByteStreamDataSource(const IByteStreamPtr& stream, size_t bufferSize = DefaultBufferSize)
			: _stream(STINGRAYKIT_REQUIRE_NOT_NULL(stream)), _buffer(bufferSize)
		{ STINGRAYKIT_CHECK(bufferSize != 0, ArgumentException("bufferSize")); }

		void Read(IDataConsumer& consumer, const ICancellationToken& token) override
		{
			const size_t read = _stream->Read(_buffer, token);

			if (read != 0)
				ConsumeAll(consumer, ConstByteData(_buffer, 0, read), token);
			else if (token)
				consumer.EndOfData(token);
		}

		void Seek(s64 offset, SeekMode mode = SeekMode::Begin)
		{ _stream->Seek(offset, mode); }

		u64 Tell() const
		{ return _stream->Tell(); }
	};
	STINGRAYKIT_DECLARE_PTR(ByteStreamDataSource);

}

#endif
