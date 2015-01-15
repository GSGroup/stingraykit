#ifndef STINGRAYKIT_IO_ICIRCULARBUFFER_H
#define STINGRAYKIT_IO_ICIRCULARBUFFER_H

#include <limits>

#include <stingraykit/log/Logger.h>
#include <stingraykit/collection/ByteData.h>
#include <stingraykit/exception.h>
#include <stingraykit/shared_ptr.h>


namespace stingray
{

	/**
	 * @addtogroup core_storage
	 * @{
	 */

	STINGRAYKIT_DECLARE_SIMPLE_EXCEPTION(BufferIsFullException, "Buffer is full!");
	STINGRAYKIT_DECLARE_SIMPLE_EXCEPTION(BufferUnderrunException, "Can't pop such a data size - it's too big!");

	class CircularDataReserver
	{
	private:
		typedef const function<void(size_t)>	OffsetShifterFunc;

		//static NamedLogger	s_logger;
		ConstByteData		_data;
		OffsetShifterFunc	_dataReleaser;

	public:
		CircularDataReserver(ConstByteData data, const OffsetShifterFunc& dataReleaser)
			: _data(data), _dataReleaser(dataReleaser)
		{ }

		~CircularDataReserver()
		{ STINGRAYKIT_TRY("Cannot release data!", _dataReleaser(_data.size())); }

		ConstByteData GetData() const
		{ return _data; }
	};
	STINGRAYKIT_DECLARE_PTR(CircularDataReserver);


	struct ICircularBuffer
	{
		virtual ~ICircularBuffer() {}

		virtual void SetLoggingEnabled() = 0;
		virtual size_t GetSize() const = 0;
		virtual size_t GetFreeSize() const = 0;
		virtual size_t GetStorageSize() const = 0;
		virtual CircularDataReserverPtr Pop(size_t size = std::numeric_limits<size_t>::max()) = 0;
		virtual void Push(const ConstByteData &data) = 0;
		virtual bool CanPush(size_t size) = 0;
		virtual void Clear() = 0;
	};
	STINGRAYKIT_DECLARE_PTR(ICircularBuffer);

	/** @} */

}


#endif
