#ifndef STINGRAY_TOOLKIT_JSONLINESSTREAM_H
#define STINGRAY_TOOLKIT_JSONLINESSTREAM_H


#include <stingray/toolkit/io/IDataSource.h>
#include <stingray/settings/ByteDataSerializer.h>


namespace stingray
{

	class JsonLinesStream
	{
	private:
		IDataConsumerPtr	_consumer;

		CancellationToken	_token;

	public:
		explicit JsonLinesStream(const IDataConsumerPtr& consumer)
			: _consumer(consumer)
		{ }

		~JsonLinesStream() { _token.Cancel(); }

		template < typename ObjectType >
		JsonLinesStream& Write(const ObjectType& object)
		{
			ByteArray buffer;

			const ConstByteArray data(ByteDataSerializer::Serialize(object, SettingsFormat::Json));
			buffer.append(data.begin(), data.end());

			const std::string LineSeparator("\r\n");
			buffer.append(LineSeparator.begin(), LineSeparator.end());

			for (size_t offset = 0; offset < buffer.size(); )
			{
				const size_t proccessed = _consumer->Process(ConstByteData(buffer, offset), _token);
				offset += proccessed;
			}

			return *this;
		}
	};

}


#endif
