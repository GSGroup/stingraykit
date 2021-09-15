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
		explicit ByteDataConsumer(ByteData destination);

		size_t Process(ConstByteData data, const ICancellationToken&) override;
		void EndOfData(const ICancellationToken&) override { _eod = true; }

		bool IsFull() const { return _destination.empty(); }
		bool IsEndOfData() const { return _eod; }
	};
	STINGRAYKIT_DECLARE_PTR(ByteDataConsumer);

}

#endif
