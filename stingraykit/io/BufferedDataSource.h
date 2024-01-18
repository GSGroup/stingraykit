// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#ifndef STINGRAYKIT_IO_BUFFEREDDATASOURCE_H
#define STINGRAYKIT_IO_BUFFEREDDATASOURCE_H

#include <stingraykit/io/IDataSource.h>
#include <stingraykit/io/SharedCircularBuffer.h>
#include <stingraykit/log/Logger.h>

namespace stingray
{

	class BufferedDataSource : public virtual IDataSource
	{
	private:
		static NamedLogger		s_logger;

		SharedCircularBufferPtr	_buffer;
		const size_t			_outputPacketSize;

	public:
		BufferedDataSource(const SharedCircularBufferPtr& buffer, size_t outputPacketSize)
			:	_buffer(STINGRAYKIT_REQUIRE_NOT_NULL(buffer)),
				_outputPacketSize(outputPacketSize)
		{
			STINGRAYKIT_CHECK(outputPacketSize != 0, ArgumentException("outputPacketSize", outputPacketSize));
			STINGRAYKIT_CHECK(SharedCircularBuffer::BufferLock(*_buffer).GetStorageSize() % outputPacketSize == 0, "Buffer size is not a multiple of output packet size!");
		}

		void Read(IDataConsumer& consumer, const ICancellationToken& token) override;

		size_t GetOutputPacketSize() const
		{ return _outputPacketSize; }
	};

}

#endif
