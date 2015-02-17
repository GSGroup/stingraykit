#ifndef STINGRAYKIT_IO_BYTEARRAYDATASOURCE_H
#define STINGRAYKIT_IO_BYTEARRAYDATASOURCE_H

#include <stingraykit/collection/ByteData.h>
#include <stingraykit/io/IDataSource.h>

namespace stingray
{

	class ByteArrayDataSource : public virtual IDataSource
	{
	private:
		ConstByteArray	_data;
		size_t			_processed;

	public:
		explicit ByteArrayDataSource(const ConstByteArray& data)
			: _data(data), _processed(0)
		{ }

		virtual void Read(IDataConsumer& consumer, const ICancellationToken& token)
		{
			if (_processed == _data.size())
			{
				consumer.EndOfData();
				return;
			}

			const ConstByteData chunk(_data, _processed);

			const size_t processed = consumer.Process(chunk, token);
			STINGRAYKIT_CHECK(processed <= chunk.size(), IndexOutOfRangeException(processed, chunk.size()));

			_processed += processed;
		}
	};

}

#endif
