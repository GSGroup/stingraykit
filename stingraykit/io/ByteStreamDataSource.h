#ifndef STINGRAYKIT_IO_BYTESTREAMDATASOURCE_H
#define STINGRAYKIT_IO_BYTESTREAMDATASOURCE_H

// Copyright (c) 2011 - 2015, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/io/IByteStream.h>
#include <stingraykit/io/IDataSource.h>

namespace stingray
{

	class ByteStreamDataSource : public IDataSource
	{
		static const size_t	DefaultReadSize = 128 * 1024;

	private:
		IByteStreamPtr		_stream;
		size_t				_readSize;
		ConstByteArray		_data;

	public:
		ByteStreamDataSource(const IByteStreamPtr &stream, size_t readSize = DefaultReadSize) : _stream(stream), _readSize(readSize) { }

		virtual void Read(IDataConsumer& consumer, const ICancellationToken& token)
		{
			if (_data.empty()) //fixme: replace by something more smart, like circular buffer
			{
				ByteArray data(_readSize);
				size_t s = _stream->Read(data.GetByteData());
				if (s == 0)
				{
					consumer.EndOfData(token);
					return;
				}
				_data = ConstByteArray(data, 0, s);
			}
			size_t processed = consumer.Process(_data.GetByteData(), token);
			STINGRAYKIT_CHECK(processed <= _data.size(), "invalid return value for data consumer");
			if (processed < _data.size())
			{
				_data = ConstByteArray(_data, processed, _data.size() - processed);
			}
			else
				_data = ConstByteArray(); //all data have been processed
		}
	};

}

#endif
