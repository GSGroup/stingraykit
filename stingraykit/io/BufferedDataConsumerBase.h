#ifndef STINGRAYKIT_IO_BUFFEREDDATACONSUMERBASE_H
#define STINGRAYKIT_IO_BUFFEREDDATACONSUMERBASE_H

// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <stingraykit/io/BithreadCircularBuffer.h>
#include <stingraykit/io/IDataSource.h>
#include <stingraykit/log/Logger.h>
#include <stingraykit/signal/signals.h>

#include <string.h>

namespace stingray
{

	class BufferedDataConsumerBase : public virtual IDataConsumer
	{
	protected:
		class ReadLock
		{
		private:
			BufferedDataConsumerBase&	_parent;

		public:
			ReadLock(BufferedDataConsumerBase& parent) : _parent(parent)
			{
				STINGRAYKIT_CHECK(!_parent._activeRead, InvalidOperationException("Simultaneous Read()!"));
				_parent._activeRead = true;
			}

			~ReadLock()
			{ _parent._activeRead = false; }
		};

	private:
		class WriteLock
		{
		private:
			BufferedDataConsumerBase&	_parent;

		public:
			WriteLock(BufferedDataConsumerBase& parent) : _parent(parent)
			{ ++_parent._activeWrites; }

			~WriteLock()
			{ --_parent._activeWrites; }
		};

	private:
		static NamedLogger		s_logger;

		bool					_discardOnOverflow;
		signal<void(size_t)>	_onOverflow;

		const size_t			_inputPacketSize;
		size_t					_requiredFreeSpace;

		bool					_activeRead;
		size_t					_activeWrites;

		Mutex					_writeMutex;

	protected:
		BithreadCircularBuffer	_buffer;
		bool					_eod;

		Mutex					_bufferMutex;
		ConditionVariable		_bufferEmpty;
		ConditionVariable		_bufferFull;

	protected:
		BufferedDataConsumerBase(bool discardOnOverflow, size_t size, size_t inputPacketSize, size_t requiredFreeSpace = 0)
			:	_discardOnOverflow(discardOnOverflow),
				_inputPacketSize(inputPacketSize),
				_requiredFreeSpace(requiredFreeSpace),
				_activeRead(false),
				_activeWrites(0),
				_buffer(size),
				_eod(false)
		{
			STINGRAYKIT_CHECK(inputPacketSize != 0, ArgumentException("inputPacketSize", inputPacketSize));
			STINGRAYKIT_CHECK(size % inputPacketSize == 0, "Buffer size is not a multiple of input packet size!");
			STINGRAYKIT_CHECK(size >= requiredFreeSpace, "Buffer size less then required free space!");
		}

		BufferedDataConsumerBase(bool discardOnOverflow, const BytesOwner& storage, size_t inputPacketSize, size_t requiredFreeSpace = 0)
			:	_discardOnOverflow(discardOnOverflow),
				_inputPacketSize(inputPacketSize),
				_requiredFreeSpace(requiredFreeSpace),
				_activeRead(false),
				_activeWrites(0),
				_buffer(storage),
				_eod(false)
		{
			STINGRAYKIT_CHECK(inputPacketSize != 0, ArgumentException("inputPacketSize", inputPacketSize));
			STINGRAYKIT_CHECK(_buffer.GetTotalSize() % inputPacketSize == 0, "Buffer size is not a multiple of input packet size!");
			STINGRAYKIT_CHECK(_buffer.GetTotalSize() >= requiredFreeSpace, "Buffer size less then required free space!");
		}

	public:
		size_t GetDataSize() const		{ MutexLock l(_bufferMutex); return _buffer.GetDataSize(); }
		size_t GetFreeSize() const		{ MutexLock l(_bufferMutex); return _buffer.GetFreeSize(); }
		size_t GetStorageSize() const	{ MutexLock l(_bufferMutex); return _buffer.GetTotalSize(); }

		/// @brief: Clears buffer completely. Warning: can't be called simultaneously with Process(...) or Read(...)
		void Clear()
		{
			MutexLock l(_bufferMutex);
			STINGRAYKIT_CHECK(!_activeRead, InvalidOperationException("Simultaneous Read() and Clear()!"));
			STINGRAYKIT_CHECK(_activeWrites == 0, InvalidOperationException("Simultaneous Process() and Clear()!"));

			_buffer.Clear();
			_eod = false;

			_bufferFull.Broadcast();
		}

		virtual size_t Process(ConstByteData data, const ICancellationToken& token)
		{
			if (data.size() % _inputPacketSize != 0)
			{
				s_logger.Error() << "Data size: " << data.size() << " is not a multiple of input packet size: " << _inputPacketSize;
				return data.size();
			}

			MutexLock l1(_writeMutex); // we need this mutex because write can be called simultaneously from several threads
			MutexLock l2(_bufferMutex);
			WriteLock wl(*this);

			BithreadCircularBuffer::Writer w = _buffer.Write();
			size_t packetized_size = w.size() / _inputPacketSize * _inputPacketSize;
			if (packetized_size == 0 || GetFreeSize() < _requiredFreeSpace)
			{
				if (_discardOnOverflow)
				{
					_onOverflow(data.size());
					return data.size();
				}

				_bufferFull.Wait(_bufferMutex, token);
				return 0;
			}

			size_t write_size = std::min(data.size(), packetized_size);
			{
				MutexUnlock ul(_bufferMutex);
				::memcpy(w.data(), data.data(), write_size);
			}

			w.Push(write_size);
			_bufferEmpty.Broadcast();

			return write_size;
		}

		virtual void EndOfData(const ICancellationToken&)
		{
			MutexLock l(_bufferMutex);
			_eod = true;
			_bufferEmpty.Broadcast();
		}

		signal_connector<void(size_t)> OnOverflow() const
		{ return _onOverflow.connector(); }
	};
}

#endif
