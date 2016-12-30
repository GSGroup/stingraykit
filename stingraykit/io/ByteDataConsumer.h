#ifndef STINGRAYKIT_IO_BYTEDATACONSUMER_H
#define STINGRAYKIT_IO_BYTEDATACONSUMER_H

#include <stingraykit/io/IDataSource.h>


namespace stingray
{

	class ByteDataConsumer : public virtual IDataConsumer
	{
	private:
		ByteData			_destination;
		bool				_eod;

	public:
		ByteDataConsumer(ByteData destination);

		bool IsFull() { return _destination.empty(); }
		bool IsEndOfData() { return _eod; }

		virtual size_t Process(ConstByteData data, const ICancellationToken&);
		virtual void EndOfData(const ICancellationToken&) { _eod = true; }
	};
	STINGRAYKIT_DECLARE_PTR(ByteDataConsumer);

}

#endif
