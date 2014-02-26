#ifndef STINGRAY_TOOLKIT_SIMPLEPACKETEDBUFFER_H
#define STINGRAY_TOOLKIT_SIMPLEPACKETEDBUFFER_H


#include <stingray/log/Logger.h>
#include <stingray/toolkit/BithreadCircularBuffer.h>
#include <stingray/toolkit/IDataSource.h>

namespace stingray
{

	class SimplePacketedBuffer : public virtual IDataSource
	{
	private:
		static NamedLogger		s_logger;
		BithreadCircularBuffer	_buffer;
		const size_t			_packetSize;

		Mutex					_bufferMutex;
		Mutex					_writeMutex;
		Mutex					_readMutex;

		WaitToken				_bufferEmpty;

	public:
		SimplePacketedBuffer(size_t size, size_t packetSize) :
			_buffer(size),
			_packetSize(packetSize)
		{ TOOLKIT_CHECK(size % _packetSize == 0, "Buffer size is not a multiple of packet size!"); }

		~SimplePacketedBuffer()
		{ }

		virtual void Read(IDataConsumer& consumer, const CancellationToken& token)
		{
			MutexLock l1(_readMutex);

			MutexLock l2(_bufferMutex);
			for (bool sent_data = false; token; sent_data = true)
			{
				BithreadCircularBuffer::Reader r = _buffer.Read();
				if (r.size() == 0)
				{
					if (sent_data)
						return;

					_bufferEmpty.Wait(_bufferMutex, token);
					continue;
				}

				TOOLKIT_CHECK(r.size() % _packetSize == 0, "Read size is not a multiple of packet size!");
				size_t processed_size = 0;
				{
					MutexUnlock ul(_bufferMutex);
					processed_size = consumer.Process(r.GetData());
				}
				TOOLKIT_CHECK(processed_size % _packetSize == 0, "Processed size is not a multiple of packet size!");
				if (processed_size == 0)
					return;
				r.Pop(processed_size);
			}
		}

		void Write(ConstByteData data)
		{
			TOOLKIT_CHECK(data.size() % _packetSize == 0, StringBuilder() % "Write size of " % data.size() % " bytes is not a multiple of packet size (" % _packetSize % " bytes)!");
			MutexLock l1(_writeMutex);

			MutexLock l2(_bufferMutex);
			while (data.size() != 0)
			{
				BithreadCircularBuffer::Writer w = _buffer.Write();
				if (w.size() == 0)
				{
					s_logger.Warning() << "Overflow: dropping " << data.size() << " bytes";
					continue;
				}

				size_t write_size = std::min(data.size(), w.size());
				TOOLKIT_CHECK(write_size % _packetSize == 0, StringBuilder() % "Selected write size: " % write_size % " is not a multiple of packet size: " % _packetSize);
				{
					MutexUnlock ul(_bufferMutex);
					std::copy(data.begin(), data.begin() + write_size, w.begin());
				}

				w.Push(write_size);
				_bufferEmpty.Broadcast();

				data = ConstByteData(data, write_size);
			}
		}
	};
	TOOLKIT_DECLARE_PTR(SimplePacketedBuffer);

}

#endif
