#include <stingraykit/io/ByteDataConsumer.h>

namespace stingray
{


	ByteDataConsumer::ByteDataConsumer(ByteData consumer)
		:	_consumer(consumer)
	{ }


	size_t ByteDataConsumer::Process(ConstByteData data, const ICancellationToken&)
	{
		const size_t size = data.size();

		STINGRAYKIT_CHECK(size <= _consumer.size(), IndexOutOfRangeException(size, _consumer.size()));

		std::copy(data.data(), data.data() + size, _consumer.data());
		_consumer = ByteData(_consumer, size);

		return size;
	}


}
