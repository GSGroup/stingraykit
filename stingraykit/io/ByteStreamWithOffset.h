#ifndef STINGRAYKIT_IO_IBYTESTREAMWITHOFFSET_H
#define STINGRAYKIT_IO_IBYTESTREAMWITHOFFSET_H

// Copyright (c) 2011 - 2015, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/io/IByteStream.h>
#include <stingraykit/string/ToString.h>

namespace stingray
{

	class ByteStreamWithOffset : public IByteStream
	{
		IByteStreamPtr	_stream;
		const s64		_offset;
		s64				_position;

	public:
		ByteStreamWithOffset(const IByteStreamPtr & stream, s64 offset) : _stream(stream), _offset(offset), _position(0)
		{
			STINGRAYKIT_CHECK(offset >= 0, ArgumentException("offset", offset));
			_stream->Seek(offset + _position);
		}

		virtual u64 Tell() const
		{
			u64 tell = _stream->Tell();
			STINGRAYKIT_CHECK(tell >= (u64)_offset, IndexOutOfRangeException(tell, _offset, 0));
			u64 position = tell - _offset;
			STINGRAYKIT_CHECK(position == (u64)_position, LogicException(StringBuilder() % "real position is " % position % ", our is " % _position));
			return position;
		}

		virtual void Seek(s64 offset, SeekMode mode = SeekMode::Begin)
		{
			s64 newPosition;

			switch(mode)
			{
			case SeekMode::Begin:
				newPosition = _offset + offset;
				break;

			case SeekMode::Current:
				newPosition = (s64)Tell() + _offset + offset;
				break;

			case SeekMode::End:
				{
					s64 oldPosition = (s64)_stream->Tell();
					_stream->Seek(0, SeekMode::End);
					s64 size = (s64)_stream->Tell();
					_stream->Seek(oldPosition);
					newPosition = size + offset;
					break;
				}

			default:
				STINGRAYKIT_THROW(NotImplementedException());
			}

			STINGRAYKIT_CHECK(newPosition >= _offset, IndexOutOfRangeException(newPosition, _offset, 0));
			_stream->Seek(newPosition);
			_position = newPosition - _offset;
		}

		virtual u64 Read(ByteData data, const ICancellationToken& token)
		{
			_stream->Seek(_offset + _position);
			u64 readed = _stream->Read(data, token);
			_position += (s64)readed;
			return readed;
		}

		virtual u64 Write(ConstByteData data, const ICancellationToken& token)
		{
			_stream->Seek(_offset + _position);
			u64 written = _stream->Write(data, token);
			_position += (s64)written;
			return written;
		}
	};

}


#endif
