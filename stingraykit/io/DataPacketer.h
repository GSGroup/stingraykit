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
		explicit ConsumerDepacketer(const IDataConsumerPtr& consumer)
			:	_lifeAssurance(consumer),
				_consumer(*_lifeAssurance)
		{ }

		explicit ConsumerDepacketer(IDataConsumer& consumer)
			:	_lifeAssurance(null),
				_consumer(consumer)
		{ }

		bool Process(const Packet<EmptyType>& packet, const ICancellationToken& token) override
		{
			ConsumeAll(_consumer, packet.GetData(), token);
			return true;
		}

		void EndOfData() override
		{ _consumer.EndOfData(DummyCancellationToken()); }
	};
	STINGRAYKIT_DECLARE_PTR(ConsumerDepacketer);


	class ConsumerPacketer : public virtual IDataConsumer
	{
		using PacketConsumer = IPacketConsumer<EmptyType>;
		STINGRAYKIT_DECLARE_PTR(PacketConsumer);

	private:
		PacketConsumerPtr	_lifeAssurance;
		PacketConsumer&		_consumer;
		optional<size_t>	_packetSize;

	public:
		explicit ConsumerPacketer(const PacketConsumerPtr& consumer, optional<size_t> packetSize = null)
			:	_lifeAssurance(consumer),
				_consumer(*_lifeAssurance),
				_packetSize(packetSize)
		{ }

		explicit ConsumerPacketer(PacketConsumer& consumer, optional<size_t> packetSize = null)
			:	_lifeAssurance(null),
				_consumer(consumer),
				_packetSize(packetSize)
		{ }

		size_t Process(ConstByteData data, const ICancellationToken& token) override
		{
			ConstByteData packetData(data, 0, _packetSize.get_value_or(data.size()));
			return _consumer.Process(Packet<EmptyType>(packetData), token) ? packetData.size() : 0;
		}

		void EndOfData(const ICancellationToken&) override
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
		explicit DataPacketer(const IDataSourcePtr& source, optional<size_t> packetSize = null)
			:	_lifeAssurance(source),
				_source(*_lifeAssurance),
				_packetSize(packetSize)
		{ }

		explicit DataPacketer(IDataSource& source, optional<size_t> packetSize = null)
			:	_lifeAssurance(null),
				_source(source),
				_packetSize(packetSize)
		{ }

		void Read(IPacketConsumer<EmptyType>& consumer, const ICancellationToken& token) override
		{
			ConsumerPacketer packeter(consumer, _packetSize);
			_source.Read(packeter, token);
		}
	};
	STINGRAYKIT_DECLARE_PTR(DataPacketer);


	class DataDepacketer : public virtual IDataSource
	{
		using PacketSource = IPacketSource<EmptyType>;
		STINGRAYKIT_DECLARE_PTR(PacketSource);

	private:
		PacketSourcePtr		_lifeAssurance;
		PacketSource&		_source;

	public:
		explicit DataDepacketer(const PacketSourcePtr& source)
			:	_lifeAssurance(source),
				_source(*_lifeAssurance)
		{ }

		explicit DataDepacketer(PacketSource& source)
			:	_lifeAssurance(null),
				_source(source)
		{ }

		void Read(IDataConsumer& consumer, const ICancellationToken& token) override
		{ ConsumerDepacketer depacketer(consumer); _source.Read(depacketer, token); }
	};
	STINGRAYKIT_DECLARE_PTR(DataDepacketer);

}

#endif
