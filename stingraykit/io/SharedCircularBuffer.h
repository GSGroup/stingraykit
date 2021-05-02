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
		class ReadLock;
		class WriteLock;

	private:
		BithreadCircularBuffer	_buffer;
		bool					_eod;
		ExceptionPtr			_exception;

		Mutex					_bufferMutex;
		ConditionVariable		_bufferEmpty;
		ConditionVariable		_bufferFull;

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
	};
	STINGRAYKIT_DECLARE_PTR(SharedCircularBuffer);


	class SharedCircularBuffer::BufferLock
	{
		friend class SharedCircularBuffer::BufferUnlock;
		friend class SharedCircularBuffer::ReadLock;
		friend class SharedCircularBuffer::WriteLock;

	private:
		SharedCircularBuffer&	_parent;
		MutexLock				_lock;

	public:
		BufferLock(SharedCircularBuffer& parent) : _parent(parent), _lock(parent._bufferMutex) { }

		size_t GetDataSize() const		{ return _parent._buffer.GetDataSize(); }
		size_t GetFreeSize() const		{ return _parent._buffer.GetFreeSize(); }
		size_t GetStorageSize() const	{ return _parent._buffer.GetTotalSize(); }

		bool IsEndOfData() const				{ return _parent._eod; }
		bool HasException() const				{ return _parent._exception.is_initialized(); }
		void RethrowExceptionIfAny() const		{ if (_parent._exception) RethrowException(_parent._exception); }

		void SetEndOfData()
		{
			STINGRAYKIT_CHECK(!_parent._exception, InvalidOperationException("Already got exception!"));

			_parent._eod = true;
			_parent._bufferEmpty.Broadcast();
		}

		void SetException(const std::exception& ex, const ICancellationToken& token)
		{
			STINGRAYKIT_CHECK(!_parent._eod, InvalidOperationException("Already got EOD!"));

			_parent._exception = MakeExceptionPtr(ex);
			_parent._bufferEmpty.Broadcast();
		}

		void Clear()
		{
			STINGRAYKIT_CHECK(!_parent._activeRead, InvalidOperationException("Simultaneous Read() and Clear()!"));
			STINGRAYKIT_CHECK(_parent._activeWrites == 0, InvalidOperationException("Simultaneous Process() and Clear()!"));

			_parent._buffer.Clear();
			_parent._eod = false;
			_parent._exception.reset();

			_parent._bufferFull.Broadcast();
		}
	};


	class SharedCircularBuffer::BufferUnlock
	{
	private:
		MutexUnlock			_unlock;

	public:
		BufferUnlock(BufferLock& lock) : _unlock(lock._lock) { }
	};


	class SharedCircularBuffer::ReadLock
	{
	private:
		SharedCircularBuffer&	_parent;

	public:
		ReadLock(BufferLock& lock) : _parent(lock._parent)
		{
			STINGRAYKIT_CHECK(!_parent._activeRead, InvalidOperationException("Simultaneous Read()!"));
			_parent._activeRead = true;
		}

		~ReadLock()
		{ _parent._activeRead = false; }

		BithreadCircularBuffer::Reader Read()
		{ return _parent._buffer.Read(); }

		ConditionWaitResult WaitEmpty(const ICancellationToken& token) const
		{ return _parent._bufferEmpty.Wait(_parent._bufferMutex, token); }

		void BroadcastFull()
		{ _parent._bufferFull.Broadcast(); }
	};


	class SharedCircularBuffer::WriteLock
	{
	private:
		SharedCircularBuffer&	_parent;

	public:
		WriteLock(BufferLock& lock) : _parent(lock._parent)
		{ ++_parent._activeWrites; }

		~WriteLock()
		{ --_parent._activeWrites; }

		BithreadCircularBuffer::Writer Write()
		{ return _parent._buffer.Write(); }

		ConditionWaitResult WaitFull(const ICancellationToken& token) const
		{ return _parent._bufferFull.Wait(_parent._bufferMutex, token); }

		void BroadcastEmpty()
		{ _parent._bufferEmpty.Broadcast(); }
	};


	class SharedWriteSynchronizer
	{
	public:
		class WriteGuard;

	private:
		Mutex						_writeMutex;
		bool						_writeAllow;
		ConditionVariable			_writeCond;

	public:
		SharedWriteSynchronizer() : _writeAllow(true) { }
	};


	class SharedWriteSynchronizer::WriteGuard
	{
	private:
		SharedWriteSynchronizer&	_parent;
		bool						_locked;

	public:
		WriteGuard(SharedWriteSynchronizer& parent) : _parent(parent), _locked(false) { }

		ConditionWaitResult Wait(const ICancellationToken& token)
		{
			{
				MutexLock l(_parent._writeMutex); // we need this mutex because write can be called simultaneously from several threads
				while (!_parent._writeAllow)
				{
					const ConditionWaitResult result = _parent._writeCond.Wait(_parent._writeMutex, token);
					if (result != ConditionWaitResult::Broadcasted)
						return result;
				}

				_parent._writeAllow = false;
			}

			_locked = true;

			return ConditionWaitResult::Broadcasted;
		}

		~WriteGuard()
		{
			if (_locked)
			{
				MutexLock l(_parent._writeMutex);
				_parent._writeAllow = true;
				_parent._writeCond.Broadcast();
			}
		}
	};

}

#endif
