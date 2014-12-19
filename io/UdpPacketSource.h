#ifndef STINGRAY_TOOLKIT_IO_UDPPACKETSOURCE_H
#define STINGRAY_TOOLKIT_IO_UDPPACKETSOURCE_H


#include <stingray/net/ISocket.h>
#include <stingray/toolkit/thread/CancellationToken.h>
#include <stingray/toolkit/thread/Thread.h>
#include <stingray/toolkit/io/BithreadCircularBuffer.h>
#include <stingray/toolkit/io/IDataSource.h>
#include <stingray/toolkit/math.h>


namespace stingray
{

	class UdpPacketSource : public virtual IPacketSource<EmptyType>
	{
		static const u64 	PollTimeout = 100;
		static const u64 	MaxUdpPacketSize = 65500;

	private:
		ISocketPtr		_socket;
		std::vector<u8>	_packetBuffer;
		size_t			_dataSize;

	public:
		UdpPacketSource(const ISocketPtr& socket, size_t maxPacketSize = MaxUdpPacketSize) :
			_socket(socket), _packetBuffer(maxPacketSize), _dataSize(0)
		{ }

		virtual void Read(IPacketConsumer<EmptyType>& consumer, const ICancellationToken& token)
		{
			if (_dataSize == 0)
			{
				_dataSize = _socket->Receive(ByteData(_packetBuffer), token);
				return;
			}

			if (consumer.Process(Packet<EmptyType>(ConstByteData(_packetBuffer, 0, _dataSize)), token))
				_dataSize = 0;
		}
	};
	TOOLKIT_DECLARE_PTR(UdpPacketSource);

	class StreamingSocketDataSource : public virtual IDataSource
	{
	private:
		ISocketPtr				_socket;
		size_t					_outputPacketSize;
		ThreadPtr				_worker;

		BithreadCircularBuffer	_buffer;
		Mutex					_mutex;

		ConditionVariable		_hasData;
		ConditionVariable		_hasSpace;

	public:
		StreamingSocketDataSource(const ISocketPtr& socket, size_t size, size_t outputPacketSize = 1) :
			_socket(socket), _outputPacketSize(outputPacketSize),
			_buffer(size)
		{
			TOOLKIT_CHECK(outputPacketSize != 0, ArgumentException("outputPacketSize", outputPacketSize));
			TOOLKIT_CHECK(size % outputPacketSize == 0, "Buffer size is not a multiple of output packet size!");

			_worker.reset(new Thread("streamingSocketDataSource", bind(&StreamingSocketDataSource::ThreadFunc, this, _1)));
		}

		~StreamingSocketDataSource()
		{ _worker.reset(); }

		virtual void Read(IDataConsumer& consumer, const ICancellationToken& token)
		{
			MutexLock l(_mutex);
			BithreadCircularBuffer::Reader r = _buffer.Read();
			size_t aligned_size = AlignDown(r.size(), _outputPacketSize);
			if (aligned_size == 0)
			{
				_hasData.Wait(_mutex, token);
				return;
			}

			size_t processed_size = 0;
			{
				MutexUnlock ul(l);
				processed_size = consumer.Process(ConstByteData(r.GetData(), 0, aligned_size), token);
			}

			r.Pop(processed_size);
			_hasSpace.Broadcast();
		}

	private:
		void ThreadFunc(const ICancellationToken& token)
		{
			while (token)
			{
				MutexLock l(_mutex);
				BithreadCircularBuffer::Writer w = _buffer.Write();
				if (w.size() == 0)
				{
					_hasSpace.Wait(_mutex, token);
					continue;
				}

				size_t received_size = 0;
				{
					MutexUnlock ul(l);
					received_size = _socket->Receive(w.GetData(), token);
				}

				w.Push(received_size);
				_hasData.Broadcast();
			}
		}
	};
	TOOLKIT_DECLARE_PTR(StreamingSocketDataSource);

}


#endif
