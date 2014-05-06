#ifndef STINGRAY_TOOLKIT_DATABUFFER_H
#define STINGRAY_TOOLKIT_DATABUFFER_H


#include <stingray/log/Logger.h>
#include <stingray/toolkit/BithreadCircularBuffer.h>
#include <stingray/toolkit/IDataSource.h>

namespace stingray
{

	class DataBufferBase : public virtual IDataConsumer
	{
	protected:
		static NamedLogger		s_logger;
		bool					_discardOnOverflow;
		BithreadCircularBuffer	_buffer;
		const size_t			_inputPacketSize;

		Mutex					_bufferMutex;
		Mutex					_writeMutex;

		WaitToken				_bufferEmpty;
		WaitToken				_bufferFull;
		bool					_eod;

	protected:
		DataBufferBase(bool discardOnOverflow, size_t size, size_t inputPacketSize) :
			_discardOnOverflow(discardOnOverflow), _buffer(size),
			_inputPacketSize(inputPacketSize), _eod(false)
		{
			TOOLKIT_CHECK(inputPacketSize != 0, ArgumentException("inputPacketSize", inputPacketSize));
			TOOLKIT_CHECK(size % inputPacketSize == 0, "Buffer size is not a multiple of input packet size!");
		}

	public:
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

			_bufferFull.Broadcast();
		}

		virtual size_t Process(ConstByteData data, const CancellationToken& token)
		{
			if (data.size() % _inputPacketSize != 0)
			{
				s_logger.Error() << "Data size: " << data.size() << " is not a multiple of input packet size: " << _inputPacketSize;
				return data.size();
			}

			MutexLock l1(_writeMutex); // we need this mutex because write can be called simultaneously from several threads
			MutexLock l2(_bufferMutex);
			BithreadCircularBuffer::Writer w = _buffer.Write();
			size_t packetized_size = w.size() / _inputPacketSize * _inputPacketSize;
			if (packetized_size == 0)
			{
				if (_discardOnOverflow)
				{
					s_logger.Warning() << "Overflow: dropping " << data.size() << " bytes";
					return data.size();
				}

				_bufferFull.Wait(_bufferMutex, token);
				return 0;
			}

			size_t write_size = std::min(data.size(), packetized_size);
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
	};


	class DataBuffer : public virtual IDataBuffer, public DataBufferBase
	{
	private:
		static NamedLogger		s_logger;
		const size_t			_outputPacketSize;

	public:
		DataBuffer(bool discardOnOverflow, size_t size, size_t inputPacketSize = 1) :
			DataBufferBase(discardOnOverflow, size, inputPacketSize), _outputPacketSize(inputPacketSize)
		{ }

		DataBuffer(bool discardOnOverflow, size_t size, size_t inputPacketSize, size_t outputPacketSize) :
			DataBufferBase(discardOnOverflow, size, inputPacketSize), _outputPacketSize(outputPacketSize)
		{
			TOOLKIT_CHECK(outputPacketSize != 0, ArgumentException("outputPacketSize", outputPacketSize));
			TOOLKIT_CHECK(size % outputPacketSize == 0, "Buffer size is not a multiple of output packet size!");
		}

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

			if (processed_size == 0)
				return;

			if (processed_size % _outputPacketSize != 0)
			{
				s_logger.Error() << "Processed size: " << processed_size << " is not a multiple of output packet size: " << _outputPacketSize;
				processed_size = packetized_size;
			}

			r.Pop(processed_size);
			_bufferFull.Broadcast();
		}
	};
	TOOLKIT_DECLARE_PTR(DataBuffer);

}

#endif
