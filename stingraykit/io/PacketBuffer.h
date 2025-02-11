#ifndef STINGRAYKIT_IO_PACKETBUFFER_H
#define STINGRAYKIT_IO_PACKETBUFFER_H

// Copyright (c) 2011 - 2025, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/io/BithreadCircularBuffer.h>
#include <stingraykit/io/IDataSource.h>
#include <stingraykit/log/Logger.h>

#include <deque>
#include <string.h>

namespace stingray
{

	template < typename MetadataType >
	class PacketBuffer final
		:	public virtual IPacketConsumer<MetadataType>,
			public virtual IPacketSource<MetadataType>
	{
		struct PacketInfo
		{
			size_t			Size;
			MetadataType	Metadata;

			PacketInfo(size_t size, const MetadataType& metadata)
				: Size(size), Metadata(metadata)
			{ }
		};

	private:
		static NamedLogger			s_logger;

		const bool					_discardOnOverflow;
		BithreadCircularBuffer		_buffer;
		std::deque<PacketInfo>		_packetQueue;

		Mutex						_bufferMutex;
		Mutex						_writeMutex;
		size_t						_paddingSize;

		ConditionVariable			_bufferEmpty;
		ConditionVariable			_bufferFull;
		bool						_eod;

	public:
		PacketBuffer(bool discardOnOverflow, size_t bufferSize)
			:	_discardOnOverflow(discardOnOverflow),
				_buffer(bufferSize),
				_paddingSize(0),
				_eod(false)
		{ }

		void Read(IPacketConsumer<MetadataType>& consumer, const ICancellationToken& token) override
		{
			MutexLock l(_bufferMutex);

			if (_packetQueue.empty())
			{
				if (_eod)
				{
					consumer.EndOfData();
					return;
				}

				_bufferEmpty.Wait(_bufferMutex, token);
				return;
			}

			BithreadCircularBuffer::Reader reader = _buffer.Read();
			if (reader.size() == _paddingSize && reader.IsBufferEnd())
			{
				reader.Pop(_paddingSize);
				_paddingSize = 0;

				reader = _buffer.Read();
			}

			const PacketInfo& packet = _packetQueue.front();
			STINGRAYKIT_CHECK(packet.Size <= reader.size(), "Not enough data in packet buffer, need: " + ToString(packet.Size) + ", got: " + ToString(reader.size()));

			bool processed = false;
			{
				MutexUnlock ul(l);
				processed = consumer.Process(Packet<MetadataType>(ConstByteData(reader.GetData(), 0, packet.Size), packet.Metadata), token);
			}

			if (!processed)
				return;

			reader.Pop(packet.Size);
			_packetQueue.pop_front();

			_bufferFull.Broadcast();
		}

		bool Process(const Packet<MetadataType>& packet, const ICancellationToken& token) override
		{
			const ConstByteData data(packet.GetData());
			STINGRAYKIT_CHECK(data.size() <= GetStorageSize(), StringBuilder() % "Packet is too big! Buffer size: " % GetStorageSize() % " packet size:" % data.size());

			MutexLock l1(_writeMutex); // we need this mutex because write can be called simultaneously from several threads
			MutexLock l2(_bufferMutex);

			BithreadCircularBuffer::Writer writer = _buffer.Write();
			const size_t paddingSize = writer.size() < data.size() && writer.IsBufferEnd() ? writer.size() : 0;

			if (_buffer.GetFreeSize() < paddingSize + data.size())
			{
				if (_discardOnOverflow)
				{
					Logger::Warning() << "Overflow: dropping " << data.size() << " bytes";
					return true;
				}
				else
				{
					_bufferFull.Wait(_bufferMutex, token);
					return false;
				}
			}

			if (paddingSize)
			{
				_paddingSize = paddingSize;
				writer.Push(paddingSize);

				writer = _buffer.Write();
			}

			{
				MutexUnlock ul2(l2);
				::memcpy(writer.data(), data.data(), data.size());
			}

			_packetQueue.emplace_back(data.size(), packet.GetMetadata());
			writer.Push(data.size());

			_bufferEmpty.Broadcast();
			return true;
		}

		void EndOfData() override
		{
			MutexLock l(_bufferMutex);
			_eod = true;
			_bufferEmpty.Broadcast();
		}

		size_t GetDataSize()			{ MutexLock l(_bufferMutex); return _buffer.GetDataSize(); }
		size_t GetFreeSize()			{ MutexLock l(_bufferMutex); return _buffer.GetFreeSize(); }
		size_t GetStorageSize() const	{ MutexLock l(_bufferMutex); return _buffer.GetTotalSize(); }

		void Clear()
		{
			MutexLock l(_bufferMutex);

			while (true)
			{
				BithreadCircularBuffer::Reader reader = _buffer.Read();
				if (reader.size() == 0)
					break;

				reader.Pop(reader.size());
			}

			_packetQueue.clear();
			_paddingSize = 0;
			_eod = false;

			_bufferFull.Broadcast();
		}
	};

}

#endif
