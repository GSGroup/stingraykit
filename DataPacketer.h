#ifndef STINGRAY_TOOLKIT_DATAPACKETER_H
#define STINGRAY_TOOLKIT_DATAPACKETER_H


#include <stingray/toolkit/IDataSource.h>

namespace stingray
{

	class DataPacketer : public virtual IPacketSource
	{
	private:
		IDataSourcePtr			_source;
		size_t					_packetSize;

	public:
		DataPacketer(const IDataSourcePtr& source, size_t packetSize) : _source(source), _packetSize(packetSize)
		{ }


		virtual void Read(IPacketConsumer& consumer, const CancellationToken& token)
		{ _source->ReadToFunction(bind(&DataPacketer::Do, this, ref(consumer), _1, _2), token); }

	private:
		size_t Do(IPacketConsumer& consumer, optional<ConstByteData> data, const CancellationToken& token)
		{
			ConstByteData packet(*data, 0, _packetSize);
			return consumer.Process(packet, token) ? _packetSize : 0;
		}
	};


	class DataDepacketer : public virtual IDataSource
	{
	private:
		IPacketSourcePtr	_source;

	public:
		DataDepacketer(const IPacketSourcePtr& source) : _source(source)
		{ }


		virtual void Read(IDataConsumer& consumer, const CancellationToken& token)
		{ _source->ReadToFunction(bind(&DataDepacketer::Do, this, ref(consumer), _1, _2), token); }

	private:
		bool Do(IDataConsumer& consumer, optional<ConstByteData> packet, const CancellationToken& token)
		{
			size_t offset = 0;
			while (token && offset < packet->size())
				offset += consumer.Process(ConstByteData(*packet, offset), token);
			return true;
		}
	};

}

#endif
