#ifndef STINGRAY_TOOLKIT_UDPPACKETSOURCE_H
#define STINGRAY_TOOLKIT_UDPPACKETSOURCE_H


#include <stingray/net/ISocket.h>
#include <stingray/threads/CancellationToken.h>
#include <stingray/threads/Thread.h>
#include <stingray/toolkit/BithreadCircularBuffer.h>
#include <stingray/toolkit/IDataSource.h>


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

		virtual void Read(IPacketConsumer<EmptyType>& consumer, const CancellationToken& token)
		{
			if (_dataSize == 0)
			{
				_dataSize = _socket->Receive(ByteData(_packetBuffer), token);
				return;
			}

			if (consumer.Process(ConstByteData(_packetBuffer, 0, _dataSize), token))
				_dataSize = 0;
		}
	};
	TOOLKIT_DECLARE_PTR(UdpPacketSource);


	class StreamingSocketDataSource : public virtual IDataSource
	{
	private:
		ISocketPtr				_socket;
		ThreadPtr				_worker;

		CancellationToken		_token;

		BithreadCircularBuffer	_buffer;
		Mutex					_mutex;

		WaitToken				_hasData;
		WaitToken				_hasSpace;

	public:
		StreamingSocketDataSource(const ISocketPtr& socket, size_t size) :
			_socket(socket),
			_buffer(size)
		{
			_worker.reset(new Thread("streamingSocketDataSource", bind(&StreamingSocketDataSource::ThreadFunc, this)));
		}

		~StreamingSocketDataSource()
		{
			_token.Cancel();
			_worker->Join();
		}

		virtual void Read(IDataConsumer& consumer, const CancellationToken& token)
		{
			MutexLock l(_mutex);
			BithreadCircularBuffer::Reader r = _buffer.Read();
			if (r.size() == 0)
			{
				_hasData.Wait(_mutex, token);
				return;
			}

			size_t processed_size = 0;
			{
				MutexUnlock ul(l);
				processed_size = consumer.Process(r.GetData(), token);
			}

			r.Pop(processed_size);
			_hasSpace.Broadcast();
		}

	private:
		void ThreadFunc()
		{
			while (_token)
			{
				MutexLock l(_mutex);
				BithreadCircularBuffer::Writer w = _buffer.Write();
				if (w.size() == 0)
				{
					_hasSpace.Wait(_mutex, _token);
					continue;
				}

				size_t received_size = 0;
				{
					MutexUnlock ul(l);
					received_size = _socket->Receive(w.GetData(), _token);
				}

				w.Push(received_size);
				_hasData.Broadcast();
			}
		}
	};
	TOOLKIT_DECLARE_PTR(StreamingSocketDataSource);

}


#endif
