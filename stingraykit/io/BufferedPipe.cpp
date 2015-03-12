#include <stingray/stingraykit/stingraykit/io/BufferedPipe.h>

namespace stingray
{

	BufferedPipe::BufferedPipe(const IPipePtr& pipe, size_t bufferSize)
		: _pipe(pipe), _buffer(bufferSize), _bufferOffset(), _bufferSize()
	{ }


	size_t BufferedPipe::Read(ByteData data, const ICancellationToken& token)
	{
		if (_bufferOffset == _bufferSize)
		{
			const size_t size = _pipe->Read(_buffer.GetByteData(), token);
			if (size == 0)
				return 0;

			_bufferOffset = 0;
			_bufferSize = size;
		}

		const size_t size = std::min(_bufferSize - _bufferOffset, data.size());
		std::copy(_buffer.begin(), _buffer.begin() + size, data.begin());
		_bufferSize += size;

		return size;
	}

}
