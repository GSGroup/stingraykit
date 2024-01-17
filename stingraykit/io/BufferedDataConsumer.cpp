// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/io/BufferedDataConsumer.h>

#include <string.h>

namespace stingray
{

	STINGRAYKIT_DEFINE_NAMED_LOGGER(BufferedDataConsumer);


	size_t BufferedDataConsumer::Process(ConstByteData data, const ICancellationToken& token)
	{
		if (data.size() % _inputPacketSize != 0)
		{
			s_logger.Error() << "Data size: " << data.size() << " is not a multiple of input packet size: " << _inputPacketSize;
			return data.size();
		}

		SharedCircularBuffer::BufferLock bl(*_buffer);
		SharedCircularBuffer::WriteLock wl(bl);

		STINGRAYKIT_CHECK(!bl.IsEndOfData(), InvalidOperationException("Already got EOD!"));
		STINGRAYKIT_CHECK(!bl.HasException(), InvalidOperationException("Already got exception!"));

		BithreadCircularBuffer::Writer w = wl.Write();
		const size_t packetizedSize = w.size() / _inputPacketSize * _inputPacketSize;
		if (packetizedSize == 0 || bl.GetFreeSize() < _requiredFreeSpace)
		{
			if (_discardOnOverflow)
			{
				_onOverflow(data.size());
				return data.size();
			}

			wl.WaitFull(token);
			return 0;
		}

		size_t writeSize = std::min(data.size(), packetizedSize);
		{
			SharedCircularBuffer::BufferUnlock bul(bl);
			::memcpy(w.data(), data.data(), writeSize);
		}

		w.Push(writeSize);
		wl.BroadcastEmpty();

		return writeSize;
	}

}
