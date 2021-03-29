// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#ifndef STINGRAYKIT_IO_BUFFEREDDATACONSUMER_H
#define STINGRAYKIT_IO_BUFFEREDDATACONSUMER_H

#include <stingraykit/io/IDataSource.h>
#include <stingraykit/io/SharedCircularBuffer.h>
#include <stingraykit/log/Logger.h>
#include <stingraykit/signal/signals.h>

#include <string.h>

namespace stingray
{

	class BufferedDataConsumer : public virtual IDataConsumer
	{
	private:
		static NamedLogger			s_logger;

		SharedCircularBufferPtr		_buffer;

		bool						_discardOnOverflow;
		signal<void(size_t)>		_onOverflow;

		const size_t				_inputPacketSize;
		size_t						_requiredFreeSpace;

		Mutex						_writeMutex;

	public:
		BufferedDataConsumer(const SharedCircularBufferPtr& buffer, bool discardOnOverflow, size_t inputPacketSize, size_t requiredFreeSpace = 0)
			:	_buffer(STINGRAYKIT_REQUIRE_NOT_NULL(buffer)),
				_discardOnOverflow(discardOnOverflow),
				_inputPacketSize(inputPacketSize),
				_requiredFreeSpace(requiredFreeSpace)
		{
			STINGRAYKIT_CHECK(inputPacketSize != 0, ArgumentException("inputPacketSize", inputPacketSize));
			STINGRAYKIT_CHECK(_buffer->_buffer.GetTotalSize() % inputPacketSize == 0, "Buffer size is not a multiple of input packet size!");
			STINGRAYKIT_CHECK(_buffer->_buffer.GetTotalSize() >= requiredFreeSpace, "Buffer size less then required free space!");
		}

		size_t Process(ConstByteData data, const ICancellationToken& token) override
		{
			if (data.size() % _inputPacketSize != 0)
			{
				s_logger.Error() << "Data size: " << data.size() << " is not a multiple of input packet size: " << _inputPacketSize;
				return data.size();
			}

			MutexLock l1(_writeMutex); // we need this mutex because write can be called simultaneously from several threads
			SharedCircularBuffer::BufferLock bl(*_buffer);
			SharedCircularBuffer::WriteLock wl(bl);

			STINGRAYKIT_CHECK(!_buffer->_eod, InvalidOperationException("Already got EOD!"));
			STINGRAYKIT_CHECK(!_buffer->_exception, InvalidOperationException("Already got exception!"));

			BithreadCircularBuffer::Writer w = wl.Write();
			size_t packetized_size = w.size() / _inputPacketSize * _inputPacketSize;
			if (packetized_size == 0 || _buffer->_buffer.GetFreeSize() < _requiredFreeSpace)
			{
				if (_discardOnOverflow)
				{
					_onOverflow(data.size());
					return data.size();
				}

				wl.WaitFull(token);
				return 0;
			}

			size_t write_size = std::min(data.size(), packetized_size);
			{
				SharedCircularBuffer::BufferUnlock ul(bl);
				::memcpy(w.data(), data.data(), write_size);
			}

			w.Push(write_size);
			wl.BroadcastEmpty();

			return write_size;
		}

		void EndOfData(const ICancellationToken&) override
		{ _buffer->SetEndOfData(); }

		signal_connector<void(size_t)> OnOverflow() const
		{ return _onOverflow.connector(); }
	};

}

#endif
