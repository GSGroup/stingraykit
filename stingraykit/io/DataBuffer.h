#ifndef STINGRAYKIT_IO_DATABUFFER_H
#define STINGRAYKIT_IO_DATABUFFER_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/io/BufferedDataConsumerBase.h>

namespace stingray
{

	class DataBuffer : public virtual IDataBuffer, public BufferedDataConsumerBase
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

		const size_t			_outputPacketSize;

	public:
		DataBuffer(bool discardOnOverflow, size_t size, Parameters parameters = Parameters())
			:	BufferedDataConsumerBase(discardOnOverflow, size, parameters.InputPacketSize, parameters.RequiredFreeSpace),
				_outputPacketSize(parameters.OutputPacketSize)
		{
			STINGRAYKIT_CHECK(parameters.OutputPacketSize != 0, ArgumentException("parameters.OutputPacketSize", parameters.OutputPacketSize));
			STINGRAYKIT_CHECK(size % parameters.OutputPacketSize == 0, "Buffer size is not a multiple of output packet size!");
		}

		DataBuffer(bool discardOnOverflow, const BytesOwner& storage, Parameters parameters = Parameters())
			:	BufferedDataConsumerBase(discardOnOverflow, storage, parameters.InputPacketSize, parameters.RequiredFreeSpace),
				_outputPacketSize(parameters.OutputPacketSize)
		{
			STINGRAYKIT_CHECK(parameters.OutputPacketSize != 0, ArgumentException("parameters.OutputPacketSize", parameters.OutputPacketSize));
			STINGRAYKIT_CHECK(_buffer.GetTotalSize() % parameters.OutputPacketSize == 0, "Buffer size is not a multiple of output packet size!");
		}

		void Read(IDataConsumer& consumer, const ICancellationToken& token) override
		{
			MutexLock l(_bufferMutex);
			ReadLock rl(*this);

			BithreadCircularBuffer::Reader r = _buffer.Read();

			size_t packetized_size = r.size() / _outputPacketSize * _outputPacketSize;
			if (packetized_size == 0)
			{
				if (_exception)
					RethrowException(_exception);
				else if (_eod)
				{
					if (r.size() != 0)
						s_logger.Warning() << "Dropping " << r.size() << " bytes from DataBuffer - end of data!";

					consumer.EndOfData(token);
					return;
				}

				_bufferEmpty.Wait(_bufferMutex, token);
				return;
			}

			size_t processed_size = 0;
			{
				MutexUnlock ul(_bufferMutex);
				processed_size = consumer.Process(ConstByteData(r.GetData(), 0, packetized_size), token);
			}

			if (processed_size == 0)
				return;

			if (processed_size % _outputPacketSize != 0)
			{
				s_logger.Error() << "Processed size: " << processed_size << " is not a multiple of output packet size: " << _outputPacketSize;
				processed_size = packetized_size;
			}

			r.Pop(processed_size);
			_bufferFull.Broadcast();
		}

		size_t GetDataSize() const override
		{ return BufferedDataConsumerBase::GetDataSize(); }

		size_t GetFreeSize() const override
		{ return BufferedDataConsumerBase::GetFreeSize(); }

		size_t GetStorageSize() const override
		{ return BufferedDataConsumerBase::GetStorageSize(); }

		bool HasEndOfDataOrException() const override
		{ return BufferedDataConsumerBase::HasEndOfDataOrException(); }

		void WaitForData(size_t threshold, const ICancellationToken& token) override
		{
			STINGRAYKIT_CHECK(threshold > 0 && threshold % _outputPacketSize == 0 && threshold < BufferedDataConsumerBase::GetStorageSize(),
					ArgumentException("threshold", threshold));

			MutexLock l(_bufferMutex);
			ReadLock rl(*this);

			while (BufferedDataConsumerBase::GetDataSize() < threshold && !_eod && !_exception)
				if (_bufferEmpty.Wait(_bufferMutex, token) != ConditionWaitResult::Broadcasted)
					break;
		}

		void SetException(const std::exception& ex, const ICancellationToken& token) override
		{ BufferedDataConsumerBase::SetException(ex, token); }

		void Clear() override
		{ BufferedDataConsumerBase::Clear(); }

		signal_connector<void(size_t)> OnOverflow() const override
		{ return BufferedDataConsumerBase::OnOverflow(); }
	};
	STINGRAYKIT_DECLARE_PTR(DataBuffer);

}

#endif
