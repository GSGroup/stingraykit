#ifndef STINGRAYKIT_IO_BUFFEREDSTREAM_H
#define STINGRAYKIT_IO_BUFFEREDSTREAM_H

// Copyright (c) 2011 - 2015, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/io/IByteStream.h>
#include <stingraykit/math.h>

namespace stingray
{

	class BufferedStream : public virtual IByteStream
	{
		IByteStreamPtr	_stream;
		ByteArray		_readBuffer;
		size_t			_readBufferSize;
		size_t			_readBufferOffset;
		s64				_currentOffset;
		size_t			_alignment;

		bool AtEnd() const
		{ return _readBufferSize && _readBufferSize < _readBuffer.size() && _readBufferOffset >= _readBufferSize; }

		size_t GetBufferSize() const
		{ return _readBufferSize - _readBufferOffset; }

		bool SeekInBuffer(s64 offset)
		{
			if (_readBufferSize && _currentOffset - (s64)_readBufferOffset <= offset && offset < _currentOffset + (s64)GetBufferSize()) //offset within buffer
			{
				_readBufferOffset += offset - _currentOffset;
				_currentOffset = offset;
				return true;
			}
			return false;
		}

		void SeekStream(s64 offset)
		{
			const s64 alignedOffset(AlignDown<s64>(offset, _alignment));
			_stream->Seek(alignedOffset);
			_currentOffset = offset;
			_readBufferSize = 0;
			_readBufferOffset = offset - alignedOffset;
		}

	public:
		static const size_t DefaultBufferSize = 4096;
		static const size_t DefaultAlignment = 1;

		explicit BufferedStream(const IByteStreamPtr& stream, size_t bufferSize = DefaultBufferSize, size_t alignment = DefaultAlignment) :
			_stream(stream), _readBuffer(bufferSize), _readBufferSize(0), _readBufferOffset(0), _currentOffset(0), _alignment(alignment)
		{ STINGRAYKIT_CHECK(bufferSize % alignment == 0, ArgumentException("alignment", alignment)); }

		virtual u64 Read(ByteData data, const ICancellationToken& token)
		{
			u64 total = 0;
			u8 *dst = data.data();
			while (total < data.size() && !AtEnd())
			{
				if (_readBufferOffset >= _readBufferSize)
				{
					if (_readBufferSize && _readBufferOffset == _readBufferSize)
						_readBufferOffset = 0;
					_readBufferSize = _stream->Read(_readBuffer.GetByteData(), token);
				}

				if (!_readBufferSize)
					break;

				u64 n = std::min<u64>(data.size() - total, GetBufferSize());
				std::copy(_readBuffer.data() + _readBufferOffset, _readBuffer.data() + _readBufferOffset + n, dst);
				_readBufferOffset += n;
				_currentOffset += n;
				dst += n;
				total += n;
			}
			return total;
		}

		virtual u64 Write(ConstByteData data, const ICancellationToken& token)
		{
			if (_readBufferOffset != _readBufferSize)
				_stream->Seek(_currentOffset);
			u64 r = _stream->Write(data, token);
			_currentOffset += r;
			SeekStream(_currentOffset);
			return r;
		}

		virtual void Seek(s64 offset, SeekMode mode = SeekMode::Begin)
		{
			switch (mode)
			{
			case SeekMode::Begin:
				if (!SeekInBuffer(offset))
					SeekStream(offset);
				break;

			case SeekMode::Current:
				Seek(_currentOffset + offset);
				break;

			case SeekMode::End:
				_stream->Seek(offset, mode);
				{
					const u64 currentOffset = _stream->Tell();
					if (SeekInBuffer(currentOffset))
						_stream->Seek(_currentOffset - _readBufferOffset);
					else
						SeekStream(currentOffset);
				}
				break;
			}
		}

		virtual u64 Tell() const
		{ return _currentOffset; }
	};
	STINGRAYKIT_DECLARE_PTR(BufferedStream);

}

#endif
