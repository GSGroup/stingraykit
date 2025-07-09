#ifndef STINGRAYKIT_IO_PACKETBUFFER_H
#define STINGRAYKIT_IO_PACKETBUFFER_H

// Copyright (c) 2011 - 2025, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/io/SharedCircularBuffer.h>
#include <stingraykit/io/IDataSource.h>
#include <stingraykit/log/Logger.h>

#include <deque>
#include <string.h>

namespace stingray
{

	template < typename MetadataType >
	class PacketBuffer final : public virtual IPacketBuffer<MetadataType>
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

		std::deque<PacketInfo>		_packetQueue;
		size_t						_paddingSize;
		SharedCircularBuffer		_buffer;
		SharedWriteSynchronizer		_writeSync;

	public:
		PacketBuffer(bool discardOnOverflow, size_t bufferSize)
			:	_discardOnOverflow(discardOnOverflow),
				_paddingSize(0),
				_buffer(bufferSize)
		{ STINGRAYKIT_CHECK(bufferSize > 0, ArgumentException("bufferSize")); }

		void Read(IPacketConsumer<MetadataType>& consumer, const ICancellationToken& token) override
		{
			SharedCircularBuffer::BufferLock bl(_buffer);
			SharedCircularBuffer::ReadLock rl(bl);

			if (_packetQueue.empty())
			{
				if (bl.IsEndOfData())
				{
					consumer.EndOfData();
					return;
				}

				rl.WaitEmpty(token);
				return;
			}

			BithreadCircularBuffer::Reader reader = rl.Read();
			if (reader.size() == _paddingSize && reader.IsBufferEnd())
			{
				reader.Pop(_paddingSize);
				_paddingSize = 0;

				reader = rl.Read();
			}

			const PacketInfo& packet = _packetQueue.front();
			STINGRAYKIT_CHECK(packet.Size <= reader.size(), LogicException(StringBuilder() % "Reader size " % reader.size() % " is lesser than packet size: " % packet.Size));

			bool processed = false;
			{
				SharedCircularBuffer::BufferUnlock ul(bl);
				processed = consumer.Process(Packet<MetadataType>(ConstByteData(reader.GetData(), 0, packet.Size), packet.Metadata), token);
			}

			if (!processed)
				return;

			reader.Pop(packet.Size);
			_packetQueue.pop_front();

			rl.BroadcastFull();
		}

		bool Process(const Packet<MetadataType>& packet, const ICancellationToken& token) override
		{
			SharedWriteSynchronizer::WriteGuard g(_writeSync);
			if (g.Wait(token) != ConditionWaitResult::Broadcasted)
				return false;

			SharedCircularBuffer::BufferLock bl(_buffer);
			SharedCircularBuffer::WriteLock wl(bl);

			STINGRAYKIT_CHECK(packet.GetSize() <= bl.GetStorageSize(), ArgumentException("packet.GetSize()", packet.GetSize()));
			STINGRAYKIT_CHECK(!bl.IsEndOfData(), InvalidOperationException("Already got EOD"));

			BithreadCircularBuffer::Writer writer = wl.Write();

			const ConstByteData data(packet.GetData());
			const size_t paddingSize = writer.size() < data.size() && writer.IsBufferEnd() ? writer.size() : 0;

			if (bl.GetFreeSize() < paddingSize + data.size())
			{
				if (_discardOnOverflow)
				{
					s_logger.Warning() << "Process: overflow " << data.size() << " bytes";
					return true;
				}

				wl.WaitFull(token);
				return false;
			}

			if (paddingSize)
			{
				_paddingSize = paddingSize;
				writer.Push(paddingSize);

				writer = wl.Write();
			}

			{
				SharedCircularBuffer::BufferUnlock ul(bl);
				::memcpy(writer.data(), data.data(), data.size());
			}

			_packetQueue.emplace_back(data.size(), packet.GetMetadata());
			writer.Push(data.size());

			wl.BroadcastEmpty();
			return true;
		}

		void EndOfData() override
		{ SharedCircularBuffer::BufferLock(_buffer).SetEndOfData(); }

		size_t GetDataSize() const override
		{ return SharedCircularBuffer::ConstBufferLock(_buffer).GetDataSize(); }

		size_t GetFreeSize() const override
		{ return SharedCircularBuffer::ConstBufferLock(_buffer).GetFreeSize(); }

		size_t GetStorageSize() const override
		{ return SharedCircularBuffer::ConstBufferLock(_buffer).GetStorageSize(); }

		void Clear() override
		{
			SharedCircularBuffer::BufferLock bl(_buffer);

			_packetQueue.clear();
			_paddingSize = 0;

			bl.Clear();
		}
	};

	template < typename MetadataType >
	STINGRAYKIT_DEFINE_NAMED_LOGGER(PacketBuffer<MetadataType>, "PacketBuffer");

}

#endif
