#ifndef STINGRAYKIT_IO_IBYTESTREAM_H
#define STINGRAYKIT_IO_IBYTESTREAM_H

// Copyright (c) 2011 - 2015, GS Group, https://github.com/GSGroup
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


	class ByteStreamWithOffset : public IByteStream
	{
		IByteStreamPtr	_stream;
		const size_t	_offset;

	public:
		ByteStreamWithOffset(const IByteStreamPtr & stream, size_t offset) : _stream(stream), _offset(offset)
		{ _stream->Seek(offset); }

		virtual u64 Tell() const
		{
			size_t tell = _stream->Tell();
			STINGRAYKIT_CHECK(tell >= _offset, "Wrong ByteStreamWithOffset state");
			return tell - _offset;
		}

		virtual void Seek(s64 offset, SeekMode mode = SeekMode::Begin)
		{ _stream->Seek(mode == SeekMode::Begin ? static_cast<s64>(_offset) + offset : offset, mode); }

		virtual u64 Read(ByteData data, const ICancellationToken& token)						{ return _stream->Read(data, token); }
		virtual u64 Write(ConstByteData data, const ICancellationToken& token)					{ return _stream->Write(data, token); }
	};

}


#endif
