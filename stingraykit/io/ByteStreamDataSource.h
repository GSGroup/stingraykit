#ifndef STINGRAYKIT_IO_BYTESTREAMDATASOURCE_H
#define STINGRAYKIT_IO_BYTESTREAMDATASOURCE_H

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
					consumer.EndOfData();
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
