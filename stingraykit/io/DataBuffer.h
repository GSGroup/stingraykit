#ifndef STINGRAYKIT_IO_DATABUFFER_H
#define STINGRAYKIT_IO_DATABUFFER_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/io/BufferedDataConsumer.h>
#include <stingraykit/io/BufferedDataSource.h>

namespace stingray
{

	class DataBuffer : public virtual IDataBuffer
	{
	public:
		struct Parameters
		{
			size_t		InputPacketSize;
			size_t		OutputPacketSize;
			size_t		RequiredFreeSpace;

			explicit Parameters(size_t inputOutputPacketSize = 1)
				: InputPacketSize(inputOutputPacketSize), OutputPacketSize(inputOutputPacketSize), RequiredFreeSpace(0)
			{ }

			Parameters& SetInputPacketSize(size_t inputPacketSize)		{ InputPacketSize = inputPacketSize; return *this; }
			Parameters& SetOutputPacketSize(size_t outputPacketSize)	{ OutputPacketSize = outputPacketSize; return *this; }
			Parameters& SetRequiredFreeSpace(size_t requiredFreeSpace)	{ RequiredFreeSpace = requiredFreeSpace; return *this; }
		};

	private:
		static NamedLogger		s_logger;

		SharedCircularBufferPtr	_buffer;
		BufferedDataConsumer	_consumer;
		BufferedDataSource		_source;

	public:
		DataBuffer(bool discardOnOverflow, size_t size, Parameters parameters = Parameters())
			:	_buffer(make_shared_ptr<SharedCircularBuffer>(size)),
				_consumer(_buffer, discardOnOverflow, parameters.InputPacketSize, parameters.RequiredFreeSpace),
				_source(_buffer, parameters.OutputPacketSize)
		{ }

		DataBuffer(bool discardOnOverflow, const BytesOwner& storage, Parameters parameters = Parameters())
			:	_buffer(make_shared_ptr<SharedCircularBuffer>(storage)),
				_consumer(_buffer, discardOnOverflow, parameters.InputPacketSize, parameters.RequiredFreeSpace),
				_source(_buffer, parameters.OutputPacketSize)
		{ }

		void Read(IDataConsumer& consumer, const ICancellationToken& token) override
		{ _source.Read(consumer, token); }

		size_t Process(ConstByteData data, const ICancellationToken& token) override
		{ return _consumer.Process(data, token); }

		void EndOfData(const ICancellationToken& token) override
		{ _consumer.EndOfData(token); }

		size_t GetDataSize() const override
		{ return SharedCircularBuffer::BufferLock(*_buffer).GetDataSize(); }

		size_t GetFreeSize() const override
		{ return SharedCircularBuffer::BufferLock(*_buffer).GetFreeSize(); }

		size_t GetStorageSize() const override
		{ return SharedCircularBuffer::BufferLock(*_buffer).GetStorageSize(); }

		bool HasEndOfDataOrException() const override
		{ return SharedCircularBuffer::BufferLock(*_buffer).HasEndOfDataOrException(); }

		void WaitForData(size_t threshold, const ICancellationToken& token) override
		{
			SharedCircularBuffer::BufferLock bl(*_buffer);

			STINGRAYKIT_CHECK(threshold > 0 && threshold % _source.GetOutputPacketSize() == 0 && threshold < bl.GetStorageSize(),
					ArgumentException("threshold", threshold));

			SharedCircularBuffer::ReadLock rl(bl);

			while (bl.GetDataSize() < threshold && !bl.HasEndOfDataOrException())
				if (rl.WaitEmpty(token) != ConditionWaitResult::Broadcasted)
					break;
		}

		void SetException(const std::exception& ex, const ICancellationToken& token) override
		{ SharedCircularBuffer::BufferLock(*_buffer).SetException(ex, token); }

		void Clear() override
		{ SharedCircularBuffer::BufferLock(*_buffer).Clear(); }

		signal_connector<void(size_t)> OnOverflow() const override
		{ return _consumer.OnOverflow(); }
	};
	STINGRAYKIT_DECLARE_PTR(DataBuffer);

}

#endif
