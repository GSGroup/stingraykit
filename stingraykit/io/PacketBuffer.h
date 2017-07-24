#ifndef STINGRAYKIT_IO_DATABUFFER_H
#define STINGRAYKIT_IO_DATABUFFER_H

// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <stingraykit/io/BithreadCircularBuffer.h>
#include <stingraykit/io/IDataSource.h>
#include <stingraykit/log/Logger.h>

#include <string.h>
#include <deque>

namespace stingray
{

	template<typename MetadataType>
	class PacketBuffer :
		public virtual IPacketConsumer<MetadataType>, public virtual IPacketSource<MetadataType>
	{
		struct PacketInfo
		{
			size_t			Size;
			MetadataType	Metadata;

			PacketInfo(size_t size, const MetadataType& md) : Size(size), Metadata(md)
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
		PacketBuffer(bool discardOnOverflow, size_t size) :
			_discardOnOverflow(discardOnOverflow), _buffer(size),
			_paddingSize(0), _eod(false)
		{ }

		size_t GetDataSize()			{ MutexLock l(_bufferMutex); return _buffer.GetDataSize(); }
		size_t GetFreeSize()			{ MutexLock l(_bufferMutex); return _buffer.GetFreeSize(); }
		size_t GetStorageSize() const	{ MutexLock l(_bufferMutex); return _buffer.GetTotalSize(); }

		void Clear()
		{
			MutexLock l(_bufferMutex);
			while (true)
			{
				BithreadCircularBuffer::Reader r = _buffer.Read();
				if (r.size() == 0)
					break;

				r.Pop(r.size());
			}
			_packetQueue.clear();
			_paddingSize = 0;
			_eod = false;

			_bufferFull.Broadcast();
		}

		virtual bool Process(const Packet<MetadataType>& packet, const ICancellationToken& token)
		{
			ConstByteData data(packet.GetData());
			STINGRAYKIT_CHECK(data.size() <= GetStorageSize(), StringBuilder() % "Packet is too big! Buffer size: " % GetStorageSize() % " packet size:" % data.size());

			MutexLock l1(_writeMutex); // we need this mutex because write can be called simultaneously from several threads
			MutexLock l2(_bufferMutex);

			BithreadCircularBuffer::Writer w = _buffer.Write();
			size_t padding_size = (w.size() < data.size() && w.IsBufferEnd()) ? w.size() : 0;
			if (_buffer.GetFreeSize() < padding_size + data.size())
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

			if (padding_size)
			{
				_paddingSize = padding_size;
				w.Push(padding_size);
				w = _buffer.Write();
			}

			{
				MutexUnlock ul(_bufferMutex);
				::memcpy(w.data(), data.data(), data.size());
			}
			PacketInfo p(data.size(), packet.GetMetadata());
			_packetQueue.push_back(p);

			w.Push(data.size());
			_bufferEmpty.Broadcast();

			return true;
		}

		virtual void EndOfData()
		{
			MutexLock l(_bufferMutex);
			_eod = true;
			_bufferEmpty.Broadcast();
		}

		virtual void Read(IPacketConsumer<MetadataType>& consumer, const ICancellationToken& token)
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

			BithreadCircularBuffer::Reader r = _buffer.Read();
			if (r.size() == _paddingSize && r.IsBufferEnd())
			{
				r.Pop(_paddingSize);
				_paddingSize = 0;
				r = _buffer.Read();
			}

			PacketInfo p = _packetQueue.front();
			STINGRAYKIT_CHECK(p.Size <= r.size(), "Not enough data in packet buffer, need: " + ToString(p.Size) + ", got: " + ToString(r.size()));
			bool processed = false;
			{
				MutexUnlock ul(_bufferMutex);
				processed = consumer.Process(Packet<MetadataType>(ConstByteData(r.GetData(), 0, p.Size), p.Metadata), token);
			}

			if (!processed)
				return;

			r.Pop(p.Size);
			_packetQueue.pop_front();
			_bufferFull.Broadcast();
		}
	};

}

#endif
