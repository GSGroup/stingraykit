#ifndef STINGRAYKIT_IO_BUFFEREDSTREAM_H
#define STINGRAYKIT_IO_BUFFEREDSTREAM_H

// Copyright (c) 2011 - 2015, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/io/IByteStream.h>

namespace stingray
{

	class BufferedStream : public virtual IByteStream
	{
		IByteStreamPtr	_stream;
		ByteArray		_readBuffer;
		size_t			_readBufferSize;
		size_t			_readBufferOffset;
		s64				_currentOffset;

		bool AtEnd() const
		{ return _readBufferSize < _readBuffer.size() && _readBufferOffset >= _readBufferSize; }

		size_t GetBufferSize() const
		{ return _readBufferSize - _readBufferOffset; }

		bool SeekInBuffer(s64 offset)
		{
			if (offset >= _currentOffset && offset < _currentOffset + (s64)GetBufferSize()) //offset within buffer
			{
				_readBufferOffset += offset - _currentOffset;
				_currentOffset = offset;
				return true;
			}
			else
				return false;
		}

	public:
		static const size_t DefaultBufferSize = 4096;

		BufferedStream(IByteStreamPtr stream, size_t bufferSize = DefaultBufferSize):
			_stream(stream), _readBuffer(bufferSize), _readBufferSize(bufferSize), _readBufferOffset(bufferSize), _currentOffset(0) {}

		virtual u64 Read(ByteData data)
		{
			u64 total = 0;
			u8 *dst = data.data();
			while(total < data.size() && !AtEnd())
			{
				if (_readBufferOffset >= _readBufferSize)
				{
					_readBufferSize = _stream->Read(_readBuffer.GetByteData());
					_readBufferOffset = 0;
				}
				u64 n = std::min<u64>(data.size(), GetBufferSize());
				std::copy(_readBuffer.data() + _readBufferOffset, _readBuffer.data() + _readBufferOffset + n, dst);
				_readBufferOffset += n;
				_currentOffset += n;
				dst += n;
				total += n;
			}
			return total;
		}

		virtual u64 Write(ConstByteData data)
		{
			if (_readBufferOffset < _readBufferSize)
			{
				_readBufferOffset = _readBufferSize; //flush read buffer
				_stream->Seek(_currentOffset);
			}
			u64 r = _stream->Write(data);
			_currentOffset += r;
			return r;
		}

		virtual void Seek(s64 offset, SeekMode mode = SeekMode::Begin)
		{
			switch(mode)
			{
				case SeekMode::Begin:
					if (SeekInBuffer(offset))
						return;
					break;
				case SeekMode::Current:
					if (SeekInBuffer(_currentOffset + offset))
						return;
					break;
				default:
					break;
			}

			_stream->Seek(offset, mode);
			_currentOffset = _stream->Tell();
			_readBufferOffset = _readBufferSize = _readBuffer.size();
		}

		virtual u64 Tell() const
		{ return _currentOffset; }
	};
	STINGRAYKIT_DECLARE_PTR(BufferedStream);

}

#endif
