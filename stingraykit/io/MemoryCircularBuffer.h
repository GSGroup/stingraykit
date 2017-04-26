#ifndef STINGRAYKIT_IO_MEMORYCIRCULARBUFFER_H
#define STINGRAYKIT_IO_MEMORYCIRCULARBUFFER_H

// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <cstring>
#include <vector>

#include <stingraykit/io/CircularBufferBase.h>

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
