#ifndef STINGRAYKIT_IO_DATAPACKETER_H
#define STINGRAYKIT_IO_DATAPACKETER_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/io/IDataSource.h>
#include <stingraykit/thread/DummyCancellationToken.h>

namespace stingray
{


	class ConsumerDepacketer : public virtual IPacketConsumer<EmptyType>
	{
	private:
		IDataConsumerPtr	_lifeAssurance;
		IDataConsumer&		_consumer;

	public:
		ConsumerDepacketer(const IDataConsumerPtr& consumer)
			:	_lifeAssurance(consumer),
				_consumer(*_lifeAssurance)
		{}


		ConsumerDepacketer(IDataConsumer& consumer)
			:	_lifeAssurance(null),
				_consumer(consumer)
		{}


		virtual bool Process(const Packet<EmptyType>& packet, const ICancellationToken& token)
		{
			ConsumeAll(_consumer, packet.GetData(), token);
			return true;
		}


		virtual void EndOfData()
		{ _consumer.EndOfData(DummyCancellationToken()); }
	};
	STINGRAYKIT_DECLARE_PTR(ConsumerDepacketer);


	class ConsumerPacketer : public virtual IDataConsumer
	{
		typedef IPacketConsumer<EmptyType> PacketConsumer;
		STINGRAYKIT_DECLARE_PTR(PacketConsumer);

	private:
		PacketConsumerPtr	_lifeAssurance;
		PacketConsumer&		_consumer;
		optional<size_t>	_packetSize;

	public:
		ConsumerPacketer(const PacketConsumerPtr& consumer, optional<size_t> packetSize = null)
			:	_lifeAssurance(consumer),
				_consumer(*_lifeAssurance),
				_packetSize(packetSize)
		{}


		ConsumerPacketer(PacketConsumer& consumer, optional<size_t> packetSize = null)
			:	_lifeAssurance(null),
				_consumer(consumer),
				_packetSize(packetSize)
		{}


		virtual size_t Process(ConstByteData data, const ICancellationToken& token)
		{
			ConstByteData packetData(data, 0, (_packetSize ? *_packetSize : data.size()));
			return _consumer.Process(Packet<EmptyType>(packetData), token) ? packetData.size() : 0;
		}


		virtual void EndOfData(const ICancellationToken&)
		{ _consumer.EndOfData(); }
	};
	STINGRAYKIT_DECLARE_PTR(ConsumerPacketer);


	class DataPacketer : public virtual IPacketSource<EmptyType>
	{
	private:
		IDataSourcePtr			_lifeAssurance;
		IDataSource&			_source;
		optional<size_t>		_packetSize;

	public:
		DataPacketer(const IDataSourcePtr& source, optional<size_t> packetSize = null)
			:	_lifeAssurance(source),
				_source(*_lifeAssurance),
				_packetSize(packetSize)
		{ }

		DataPacketer(IDataSource& source, optional<size_t> packetSize = null)
			:	_lifeAssurance(null),
				_source(source),
				_packetSize(packetSize)
		{ }

		virtual void Read(IPacketConsumer<EmptyType>& consumer, const ICancellationToken& token)
		{
			ConsumerPacketer packeter(consumer, _packetSize);
			_source.Read(packeter, token);
		}
	};
	STINGRAYKIT_DECLARE_PTR(DataPacketer);


	class DataDepacketer : public virtual IDataSource
	{
		typedef IPacketSource<EmptyType> PacketSource;
		STINGRAYKIT_DECLARE_PTR(PacketSource);

	private:
		PacketSourcePtr		_lifeAssurance;
		PacketSource&		_source;

	public:
		DataDepacketer(const PacketSourcePtr& source)
			:	_lifeAssurance(source),
				_source(*_lifeAssurance)
		{ }

		DataDepacketer(PacketSource& source)
			:	_lifeAssurance(null),
				_source(source)
		{ }

		virtual void Read(IDataConsumer& consumer, const ICancellationToken& token)
		{ ConsumerDepacketer depacketer(consumer); _source.Read(depacketer, token); }
	};
	STINGRAYKIT_DECLARE_PTR(DataDepacketer);

}

#endif
