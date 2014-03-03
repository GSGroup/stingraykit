#ifndef STINGRAY_TOOLKIT_DATASOURCEFROMSOCKET_H
#define STINGRAY_TOOLKIT_DATASOURCEFROMSOCKET_H


#include <stingray/net/ISocket.h>
#include <stingray/toolkit/IDataSource.h>

namespace stingray
{

	struct DataSourceFromSocket : public virtual IDataSource
	{
		static const size_t	MaxPacketSize = 1500;
		static const u64 	PollTimeout = 100;

	private:
		ISocketPtr		_socket;
		std::vector<u8>	_packetBuffer;
		size_t			_dataOffset;
		size_t			_dataSize;

	public:
		DataSourceFromSocket(const ISocketPtr& socket) :
			_socket(socket), _packetBuffer(MaxPacketSize), _dataOffset(0), _dataSize(0)
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
			size_t processed_size = consumer.Process(data);
			_dataOffset += processed_size;
			_dataSize -= processed_size;
		}
	};

}

#endif
