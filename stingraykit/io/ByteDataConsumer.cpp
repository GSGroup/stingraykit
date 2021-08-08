#include <stingraykit/io/ByteDataConsumer.h>

#include <string.h>

namespace stingray
{

	ByteDataConsumer::ByteDataConsumer(ByteData destination)
		:	_destination(destination),
			_eod(false)
	{ }


	size_t ByteDataConsumer::Process(ConstByteData data, const ICancellationToken&)
	{
		const size_t size = std::min(data.size(), _destination.size());
		memcpy(_destination.data(), data.data(), size);
		_destination = ByteData(_destination, size);

		return data.size();
	}

}
