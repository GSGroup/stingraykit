#include <stingraykit/io/BufferedPipeReader.h>

namespace stingray
{

	BufferedPipeReader::BufferedPipeReader(const IPipePtr& pipe, size_t bufferSize)
		: _pipe(pipe), _buffer(bufferSize)
	{ }


	size_t BufferedPipeReader::Read(ByteData data, const ICancellationToken& token)
	{
		if (_buffer.GetSize() == 0)
		{
			ByteArray buffer_(_buffer.GetFreeSize());

			const size_t read = _pipe->Read(buffer_.GetByteData(), token); // TODO: read directly into buffer

			_buffer.Push(ConstByteData(buffer_, read));
		}

		const CircularDataReserverPtr reserver = _buffer.Pop(data.size());

		const ConstByteData data_ = reserver->GetData();
		std::copy(data_.begin(), data_.end(), data.begin());

		return data_.size();
	}


	u8 BufferedPipeReader::ReadByte(const ICancellationToken& token)
	{
		u8 result;
		STINGRAYKIT_CHECK(Read(ByteData(&result, sizeof(result)), token) == sizeof(result), PipeClosedException());
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
