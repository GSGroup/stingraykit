#include <stingraykit/io/BufferedPipeReader.h>

namespace stingray
{

	BufferedPipeReader::BufferedPipeReader(const IPipePtr& pipe, size_t bufferSize)
		: _pipe(pipe), _buffer(bufferSize), _bufferedDataBegin(_buffer.end()), _bufferedDataEnd(_buffer.end())
	{ }


	size_t BufferedPipeReader::Read(ByteData data, const ICancellationToken& token)
	{
		FillBufferIfNecessary(token, false);

		const size_t size = std::min(data.size(), (size_t)std::distance(_bufferedDataBegin, _bufferedDataEnd));
		std::copy(_buffer.begin(), _buffer.begin() + size, data.begin());
		std::advance(_bufferedDataBegin, size);

		return size;
	}


	u8 BufferedPipeReader::PeekByte(const ICancellationToken& token)
	{
		FillBufferIfNecessary(token, true);
		return *_bufferedDataBegin;
	}


	u8 BufferedPipeReader::ReadByte(const ICancellationToken& token)
	{
		FillBufferIfNecessary(token, true);
		return *_bufferedDataBegin++;
	}


	std::string BufferedPipeReader::ReadLine(const ICancellationToken& token)
	{
		std::string result;
		for (u8 byte = ReadByte(token); ; byte = ReadByte(token))
		{
			if (byte == '\n')
				return result;

			if (byte == '\r')
			{
				if (PeekByte(token) == '\n')
					ReadByte(token);
				return result;
			}

			result.push_back(byte);
		}
		return result;
	}


	void BufferedPipeReader::FillBufferIfNecessary(const ICancellationToken& token, bool throwOperationCancelledException)
	{
		if (_bufferedDataBegin == _bufferedDataEnd)
		{
			const size_t read = _pipe->Read(_buffer.GetByteData(), token);
			if (read == 0)
				STINGRAYKIT_CHECK(!throwOperationCancelledException, OperationCanceledException());

			_bufferedDataBegin = _buffer.begin();
			_bufferedDataEnd = _buffer.begin() + read;
		}
	}

}
