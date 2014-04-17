#ifndef STINGRAY_TOOLKIT_DATABUFFER_H
#define STINGRAY_TOOLKIT_DATABUFFER_H


#include <stingray/log/Logger.h>
#include <stingray/toolkit/BithreadCircularBuffer.h>
#include <stingray/toolkit/IDataSource.h>

namespace stingray
{

	class DataBuffer : public virtual IDataBuffer
	{
	private:
		static NamedLogger		s_logger;
		BithreadCircularBuffer	_buffer;
		const size_t			_inputPacketSize;
		const size_t			_outputPacketSize;

		Mutex					_bufferMutex;
		Mutex					_writeMutex;

		WaitToken				_bufferEmpty;
		bool					_eod;

	public:
		DataBuffer(size_t size, size_t inputPacketSize = 1) :
			_buffer(size), _inputPacketSize(inputPacketSize), _outputPacketSize(inputPacketSize), _eod(false)
		{ CheckSizes(size, inputPacketSize, inputPacketSize); }

		DataBuffer(size_t size, size_t inputPacketSize, size_t outputPacketSize) :
			_buffer(size), _inputPacketSize(inputPacketSize), _outputPacketSize(outputPacketSize), _eod(false)
		{ CheckSizes(size, inputPacketSize, outputPacketSize); }

		virtual void Read(IDataConsumer& consumer, const CancellationToken& token)
		{
			MutexLock l(_bufferMutex);

			BithreadCircularBuffer::Reader r = _buffer.Read();

			size_t packetized_size = r.size() / _outputPacketSize * _outputPacketSize;
			if (packetized_size == 0)
			{
				if (_eod)
				{
					if (r.size() != 0)
						Logger::Warning() << "Dropping " << r.size() << " bytes from DataBuffer - end of data!";
					consumer.EndOfData();
					return;
				}

				_bufferEmpty.Wait(_bufferMutex, token);
				return;
			}

			size_t processed_size = 0;
			{
				MutexUnlock ul(_bufferMutex);
				processed_size = consumer.Process(ConstByteData(r.GetData(), 0, packetized_size), token);
			}
			TOOLKIT_CHECK(processed_size % _outputPacketSize == 0, "Processed size is not a multiple of output packet size!");
			if (processed_size == 0)
				return;
			r.Pop(processed_size);
		}

		void Clear()
		{
			MutexLock l(_bufferMutex);
			while (true)
			{
				BithreadCircularBuffer::Reader r = _buffer.Read();
				if (r.size() == 0)
					return;

				r.Pop(r.size());
			}
			_eod = false;
		}

		virtual size_t Process(ConstByteData data, const CancellationToken& token)
		{
			TOOLKIT_CHECK(data.size() % _inputPacketSize == 0, StringBuilder() % "Write size of " % data.size() % " bytes is not a multiple of input packet size (" % _inputPacketSize % " bytes)!");
			MutexLock l1(_writeMutex); // we need this mutex because write can be called simultaneously from several threads

			MutexLock l2(_bufferMutex);

			BithreadCircularBuffer::Writer w = _buffer.Write();
			if (w.size() == 0)
			{
				s_logger.Warning() << "Overflow: dropping " << data.size() << " bytes";
				return data.size();
			}

			size_t write_size = std::min(data.size(), w.size());
			TOOLKIT_CHECK(write_size % _inputPacketSize == 0, StringBuilder() % "Selected write size: " % write_size % " is not a multiple of input packet size: " % _inputPacketSize);
			{
				MutexUnlock ul(_bufferMutex);
				std::copy(data.begin(), data.begin() + write_size, w.begin());
			}

			w.Push(write_size);
			_bufferEmpty.Broadcast();

			return write_size;
		}

		virtual void EndOfData()
		{
			MutexLock l(_bufferMutex);
			_eod = true;
			_bufferEmpty.Broadcast();
		}

	private:
		void CheckSizes(size_t bufferSize, size_t inputPacketSize, size_t outputPacketSize)
		{
			TOOLKIT_CHECK(inputPacketSize != 0, ArgumentException("inputPacketSize", inputPacketSize));
			TOOLKIT_CHECK(outputPacketSize != 0, ArgumentException("outputPacketSize", outputPacketSize));

			TOOLKIT_CHECK(bufferSize % inputPacketSize == 0, "Buffer size is not a multiple of input packet size!");
			TOOLKIT_CHECK(bufferSize % outputPacketSize == 0, "Buffer size is not a multiple of output packet size!");
		}
	};
	TOOLKIT_DECLARE_PTR(DataBuffer);

}

#endif
