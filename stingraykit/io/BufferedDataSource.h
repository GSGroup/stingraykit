#ifndef STINGRAYKIT_IO_BUFFEREDDATASOURCE_H
#define STINGRAYKIT_IO_BUFFEREDDATASOURCE_H

#include <stingraykit/io/IDataSource.h>
#include <stingraykit/io/SharedCircularBuffer.h>
#include <stingraykit/log/Logger.h>

namespace stingray
{

	class BufferedDataSource : public virtual IDataSource
	{
	private:
		static NamedLogger		s_logger;

		SharedCircularBufferPtr	_buffer;
		const size_t			_outputPacketSize;

	public:
		BufferedDataSource(const SharedCircularBufferPtr& buffer, size_t outputPacketSize)
			:	_buffer(STINGRAYKIT_REQUIRE_NOT_NULL(buffer)),
				_outputPacketSize(outputPacketSize)
		{
			STINGRAYKIT_CHECK(outputPacketSize != 0, ArgumentException("outputPacketSize", outputPacketSize));
			STINGRAYKIT_CHECK(SharedCircularBuffer::BufferLock(*_buffer).GetStorageSize() % outputPacketSize == 0, "Buffer size is not a multiple of output packet size!");
		}

		void Read(IDataConsumer& consumer, const ICancellationToken& token) override;

		size_t GetOutputPacketSize() const
		{ return _outputPacketSize; }
	};

}

#endif
