#include <stingraykit/io/BufferedPipeReader.h>

namespace stingray
{

	BufferedPipeReader::BufferedPipeReader(const IPipePtr& pipe, size_t bufferSize)
		: _pipe(pipe), _buffer(bufferSize), _bufferedDataOffset(0), _bufferedDataLength(0)
	{ }


	size_t BufferedPipeReader::Read(ByteData data, const ICancellationToken& token)
	{
		if (_bufferedDataOffset == _bufferedDataLength)
		{
			_bufferedDataLength = _pipe->Read(_buffer.GetByteData(), token);
			_bufferedDataOffset = 0;
		}

		const size_t read = std::min(data.size(), _bufferedDataLength - _bufferedDataOffset);
		std::copy(_buffer.begin(), _buffer.begin() + read, data.begin());
		_bufferedDataOffset += read;

		return read;
	}


	u8 BufferedPipeReader::ReadByte(const ICancellationToken& token)
	{
		u8 result;
		STINGRAYKIT_CHECK(Read(ByteData(&result, sizeof(result)), token) == sizeof(result), OperationCanceledException());
		return result;
	}


	std::string BufferedPipeReader::ReadLine(const ICancellationToken& token)
	{
		std::string result;
		for (u8 byte = ReadByte(token); byte != '\n'; byte = ReadByte(token))
			result.push_back(byte);
		return result;
	}

}
