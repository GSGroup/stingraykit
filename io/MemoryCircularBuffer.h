#ifndef STINGRAYKIT_IO_MEMORYCIRCULARBUFFER_H
#define STINGRAYKIT_IO_MEMORYCIRCULARBUFFER_H

#include <cstring>
#include <vector>

#include <stingray/toolkit/io/CircularBufferBase.h>

namespace stingray
{

	/**
	 * @addtogroup core_storage
	 * @{
	 */

	template <bool DiscardOnOverflow = false>
	class MemoryCircularBuffer : public CircularBufferBase<DiscardOnOverflow>
	{
		typedef CircularBufferBase<DiscardOnOverflow> BaseType;
		typedef typename BaseType::DataType DataType;
		typedef std::vector<DataType> ContainerType;

		ContainerType _container;

		virtual void WriteToStorage(size_t offset, ConstByteData::iterator begin, ConstByteData::iterator end)
		{
			//std::copy(&(*begin), &(*begin) + std::distance(begin, end), _container.begin() + offset);
			::memcpy(&_container[0] + offset, &(*begin), std::distance(begin, end));
		}

		virtual ConstByteData ReadStorage(size_t offset, size_t size) const
		{
			return ConstByteData(&_container[offset], size);
		}

	public:
		MemoryCircularBuffer(size_t size): _container(size) {}

		size_t GetStorageSize() const { return _container.size(); }
	};

	/** @} */

}


#endif
