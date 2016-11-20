#ifndef STINGRAYKIT_IO_BYTEDATACONSUMER_H
#define STINGRAYKIT_IO_BYTEDATACONSUMER_H

#include <stingraykit/io/IDataSource.h>


namespace stingray
{

	class ByteDataConsumer : public virtual IDataConsumer
	{
	private:
		ByteData			_consumer;

	public:
		ByteDataConsumer(ByteData consumer);

		virtual size_t Process(ConstByteData data, const ICancellationToken&);
		virtual void EndOfData(const ICancellationToken&) { }
	};
	STINGRAYKIT_DECLARE_PTR(ByteDataConsumer);

}

#endif
