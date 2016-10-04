#ifndef STINGRAYKIT_IO_DATAPACKETER_H
#define STINGRAYKIT_IO_DATAPACKETER_H

// Copyright (c) 2011 - 2015, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <stingraykit/io/IDataSource.h>

namespace stingray
{

	class DataPacketer : public virtual IPacketSource<EmptyType>
	{
	private:
		IDataSourcePtr			_lifeAssurance;
		IDataSource&				_source;
		size_t					_packetSize;

	public:
		DataPacketer(const IDataSourcePtr& source, size_t packetSize)
			:	_lifeAssurance(source),
				_source(*source),
				_packetSize(packetSize)
		{ }

		DataPacketer(IDataSource& source, size_t packetSize)
			:	_lifeAssurance(null),
				_source(source),
				_packetSize(packetSize)
		{ }

		virtual void Read(IPacketConsumer<EmptyType>& consumer, const ICancellationToken& token)
		{ _source.ReadToFunction(bind(&DataPacketer::Do, this, ref(consumer), _1, _2), token); }

	private:
		size_t Do(IPacketConsumer<EmptyType>& consumer, ConstByteData data, const ICancellationToken& token)
		{
			ConstByteData packet(data, 0, _packetSize);
			return consumer.Process(Packet<EmptyType>(packet), token) ? _packetSize : 0;
		}
	};


	class DataDepacketer : public virtual IDataSource
	{
		typedef IPacketSource<EmptyType> PacketSource;
		STINGRAYKIT_DECLARE_PTR(PacketSource);

	private:
		PacketSourcePtr		_lifeAssurance;
		PacketSource&		_source;
		const size_t 		_startOffset;

	public:
		DataDepacketer(const PacketSourcePtr& source, size_t startOffset = 0)
			:	_lifeAssurance(source),
				_source(*source),
				_startOffset(startOffset)
		{ }

		DataDepacketer(PacketSource& source, size_t startOffset = 0)
			:	_lifeAssurance(null),
				_source(source),
				_startOffset(startOffset)
		{ }

		virtual void Read(IDataConsumer& consumer, const ICancellationToken& token)
		{ _source.ReadToFunction(bind(&DataDepacketer::Do, this, ref(consumer), _1, _2), token); }

	private:
		bool Do(IDataConsumer& consumer, const Packet<EmptyType>& packet, const ICancellationToken& token)
		{
			size_t offset = _startOffset;
			while (token && offset < packet.GetSize())
				offset += consumer.Process(ConstByteData(packet.GetData(), offset), token);
			return true;
		}
	};

}

#endif
