// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/io/BufferedDataSource.h>

namespace stingray
{

	STINGRAYKIT_DEFINE_NAMED_LOGGER(BufferedDataSource);


	void BufferedDataSource::Read(IDataConsumer& consumer, const ICancellationToken& token)
	{
		SharedCircularBuffer::BufferLock bl(*_buffer);
		SharedCircularBuffer::ReadLock rl(bl);

		BithreadCircularBuffer::Reader r = rl.Read();

		const size_t packetizedSize = r.size() / _outputPacketSize * _outputPacketSize;
		if (packetizedSize == 0)
		{
			bl.RethrowExceptionIfAny();
			if (bl.IsEndOfData())
			{
				if (r.size() != 0)
					s_logger.Warning() << "Dropping " << r.size() << " bytes from DataBuffer - end of data!";

				consumer.EndOfData(token);
				return;
			}

			rl.WaitEmpty(token);
			return;
		}

		size_t processedSize = 0;
		{
			SharedCircularBuffer::BufferUnlock ul(bl);
			processedSize = consumer.Process(ConstByteData(r.GetData(), 0, packetizedSize), token);
		}

		if (processedSize == 0)
			return;

		if (processedSize % _outputPacketSize != 0)
		{
			s_logger.Error() << "Processed size: " << processedSize << " is not a multiple of output packet size: " << _outputPacketSize;
			processedSize = packetizedSize;
		}

		r.Pop(processedSize);
		rl.BroadcastFull();
	}

}
