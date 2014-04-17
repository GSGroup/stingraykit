#ifndef STINGRAY_TOOLKIT_DATASOURCEFROMSOCKET_H
#define STINGRAY_TOOLKIT_DATASOURCEFROMSOCKET_H


#include <stingray/net/ISocket.h>
#include <stingray/threads/CancellationToken.h>
#include <stingray/threads/Thread.h>
#include <stingray/toolkit/BithreadCircularBuffer.h>
#include <stingray/toolkit/IDataSource.h>

namespace stingray
{

	struct DataSourceFromSocket : public virtual IDataSource
	{
		static const u64 	PollTimeout = 100;

	private:
		ISocketPtr		_socket;
		std::vector<u8>	_packetBuffer;
		size_t			_dataOffset;
		size_t			_dataSize;

	public:
		DataSourceFromSocket(const ISocketPtr& socket, size_t maxPacketSize) :
			_socket(socket), _packetBuffer(maxPacketSize), _dataOffset(0), _dataSize(0)
		{ }

		virtual void Read(IDataConsumer& consumer, const CancellationToken& token)
		{
			while (_dataSize == 0)
			{
				if (!token)
					return;

				if (!_socket->Poll(PollTimeout, SelectMode::Read))
					continue;

				_dataOffset = 0;
				_dataSize = _socket->Receive(&_packetBuffer[0], _packetBuffer.size());
			}

			ConstByteData data(&_packetBuffer[_dataOffset], _dataSize);
			size_t processed_size = consumer.Process(data, token);
			_dataOffset += processed_size;
			_dataSize -= processed_size;
		}
	};


	class StreamingSocketDataSource : public virtual IDataSource
	{
	private:
		ISocketPtr				_socket;
		ThreadPtr				_worker;

		CancellationToken		_token;

		BithreadCircularBuffer	_buffer;
		Mutex					_mutex;

		WaitToken				_bufferEmpty;
		WaitToken				_bufferFull;

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
				_bufferEmpty.Wait(_mutex, token);
				return;
			}

			size_t processed_size = 0;
			{
				MutexUnlock ul(l);
				processed_size = consumer.Process(r.GetData(), token);
			}

			r.Pop(processed_size);
			_bufferFull.Broadcast();
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
					_bufferFull.Wait(_mutex, _token);
					continue;
				}

				size_t received_size = 0;
				{
					MutexUnlock ul(l);
					const int PollTimeout = 100;
					if (!_socket->Poll(PollTimeout, SelectMode::Read))
						continue;

					received_size = _socket->Receive(w.GetData().data(), w.GetData().size());
				}

				w.Push(received_size);
				_bufferEmpty.Broadcast();
			}
		}
	};
	TOOLKIT_DECLARE_PTR(StreamingSocketDataSource);

}

#endif
