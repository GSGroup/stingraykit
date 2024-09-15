#ifndef STINGRAYKIT_IO_BUFFEREDDATACONSUMER_H
#define STINGRAYKIT_IO_BUFFEREDDATACONSUMER_H

// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/io/IDataSource.h>
#include <stingraykit/io/SharedCircularBuffer.h>
#include <stingraykit/log/Logger.h>
#include <stingraykit/signal/signals.h>

namespace stingray
{

	class BufferedDataConsumer : public virtual IDataConsumer
	{
	public:
		using OnOverflowSignature = void (size_t);

	private:
		static NamedLogger				s_logger;

		SharedCircularBufferPtr			_buffer;

		bool							_discardOnOverflow;
		signal<OnOverflowSignature>		_onOverflow;

		const size_t					_inputPacketSize;
		size_t							_requiredFreeSpace;

	public:
		BufferedDataConsumer(const SharedCircularBufferPtr& buffer, bool discardOnOverflow, size_t inputPacketSize, size_t requiredFreeSpace = 0)
			:	_buffer(STINGRAYKIT_REQUIRE_NOT_NULL(buffer)),
				_discardOnOverflow(discardOnOverflow),
				_inputPacketSize(inputPacketSize),
				_requiredFreeSpace(requiredFreeSpace)
		{
			STINGRAYKIT_CHECK(inputPacketSize != 0, ArgumentException("inputPacketSize", inputPacketSize));
			const size_t totalSize = SharedCircularBuffer::BufferLock(*_buffer).GetStorageSize();
			STINGRAYKIT_CHECK(totalSize % inputPacketSize == 0, "Buffer size is not a multiple of input packet size!");
			STINGRAYKIT_CHECK(totalSize >= requiredFreeSpace, "Buffer size less then required free space!");
		}

		size_t Process(ConstByteData data, const ICancellationToken& token) override;

		void EndOfData(const ICancellationToken&) override
		{ SharedCircularBuffer::BufferLock(*_buffer).SetEndOfData(); }

		signal_connector<OnOverflowSignature> OnOverflow() const
		{ return _onOverflow.connector(); }
	};

}

#endif
