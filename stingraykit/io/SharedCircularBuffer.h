#ifndef STINGRAYKIT_IO_SHAREDCIRCULARBUFFER_H
#define STINGRAYKIT_IO_SHAREDCIRCULARBUFFER_H
// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/io/BithreadCircularBuffer.h>

#include <stingraykit/thread/ConditionVariable.h>
#include <stingraykit/ExceptionPtr.h>

namespace stingray
{

	class SharedCircularBuffer
	{
	public:
		class BufferLock;
		class BufferUnlock;

		class ReadLock
		{
		private:
			SharedCircularBuffer&	_parent;

		public:
			ReadLock(SharedCircularBuffer& parent) : _parent(parent)
			{
				STINGRAYKIT_CHECK(!_parent._activeRead, InvalidOperationException("Simultaneous Read()!"));
				_parent._activeRead = true;
			}

			~ReadLock()
			{ _parent._activeRead = false; }
		};

		class WriteLock
		{
		private:
			SharedCircularBuffer&	_parent;

		public:
			WriteLock(SharedCircularBuffer& parent) : _parent(parent)
			{ ++_parent._activeWrites; }

			~WriteLock()
			{ --_parent._activeWrites; }
		};

	public:
		BithreadCircularBuffer	_buffer;
		bool					_eod;
		ExceptionPtr			_exception;

		Mutex					_bufferMutex;
		ConditionVariable		_bufferEmpty;
		ConditionVariable		_bufferFull;

	private:
		bool					_activeRead;
		size_t					_activeWrites;

	public:
		explicit SharedCircularBuffer(size_t size)
			:	_buffer(size),
				_eod(false),
				_activeRead(false),
				_activeWrites(0)
		{ }

		explicit SharedCircularBuffer(const BytesOwner& storage)
			:	_buffer(storage),
				_eod(false),
				_activeRead(false),
				_activeWrites(0)
		{ }

		size_t GetDataSize() const		{ MutexLock l(_bufferMutex); return _buffer.GetDataSize(); }
		size_t GetFreeSize() const		{ MutexLock l(_bufferMutex); return _buffer.GetFreeSize(); }
		size_t GetStorageSize() const	{ MutexLock l(_bufferMutex); return _buffer.GetTotalSize(); }

		bool HasEndOfDataOrException() const	{ MutexLock l(_bufferMutex); return _eod || _exception; }

		void SetEndOfData()
		{
			MutexLock l(_bufferMutex);

			STINGRAYKIT_CHECK(!_exception, InvalidOperationException("Already got exception!"));

			_eod = true;
			_bufferEmpty.Broadcast();
		}

		void SetException(const std::exception& ex, const ICancellationToken& token)
		{
			MutexLock l(_bufferMutex);

			STINGRAYKIT_CHECK(!_eod, InvalidOperationException("Already got EOD!"));

			_exception = MakeExceptionPtr(ex);
			_bufferEmpty.Broadcast();
		}

		void Clear()
		{
			MutexLock l(_bufferMutex);
			STINGRAYKIT_CHECK(!_activeRead, InvalidOperationException("Simultaneous Read() and Clear()!"));
			STINGRAYKIT_CHECK(_activeWrites == 0, InvalidOperationException("Simultaneous Process() and Clear()!"));

			_buffer.Clear();
			_eod = false;
			_exception.reset();

			_bufferFull.Broadcast();
		}
	};
	STINGRAYKIT_DECLARE_PTR(SharedCircularBuffer);


	class SharedCircularBuffer::BufferLock
	{
		friend class SharedCircularBuffer::BufferUnlock;

	private:
		SharedCircularBuffer&	_parent;
		MutexLock				_lock;

	public:
		BufferLock(SharedCircularBuffer& parent) : _parent(parent), _lock(parent._bufferMutex) { }
	};


	class SharedCircularBuffer::BufferUnlock
	{
	private:
		MutexUnlock			_unlock;

	public:
		BufferUnlock(BufferLock& lock) : _unlock(lock._lock) { }
	};

}

#endif
