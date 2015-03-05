#ifndef STINGRAYKIT_IO_BUFFEREDPIPEREADER_H
#define STINGRAYKIT_IO_BUFFEREDPIPEREADER_H

#include <stingraykit/io/IPipe.h>
#include <stingraykit/io/MemoryCircularBuffer.h>

namespace stingray
{

	class BufferedPipeReader
	{
		static const size_t DefaultBufferSize = 4096;

	private:
		IPipePtr						_pipe;
		MemoryCircularBuffer<false>		_buffer;

	public:
		explicit BufferedPipeReader(const IPipePtr& pipe, size_t bufferSize = DefaultBufferSize);

		size_t Read(ByteData data, const ICancellationToken& token);

		u8 ReadByte(const ICancellationToken& token);
		std::string ReadLine(const ICancellationToken& token);
	};

}

#endif
