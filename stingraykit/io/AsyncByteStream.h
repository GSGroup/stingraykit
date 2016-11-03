#ifndef STINGRAYKIT_IO_ASYNCBYTESTREAM_H
#define STINGRAYKIT_IO_ASYNCBYTESTREAM_H

// Copyright (c) 2011 - 2015, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/diagnostics/ExecutorsProfiler.h>
#include <stingraykit/diagnostics/Profiler.h>
#include <stingraykit/function/bind.h>
#include <stingraykit/io/BithreadCircularBuffer.h>
#include <stingraykit/io/ISyncableByteStream.h>
#include <stingraykit/log/Logger.h>
#include <stingraykit/thread/ConditionVariable.h>
#include <stingraykit/thread/Thread.h>
#include <stingraykit/thread/atomic.h>
#include <stingraykit/optional.h>
#include <stingraykit/toolkit.h>

#include <deque>
#include <string.h>

namespace stingray
{

	class AsyncByteStream : public ISyncableByteStream
	{
	private:
		static const size_t DefaultBufferSize = 1 * 1024 * 1024;

	public:
		class Config
		{
			size_t					_bufferSize;
			bool					_nonBlockingSync;

		public:
			Config()
				:	_bufferSize(DefaultBufferSize),
					_nonBlockingSync(false)
			{ }

			Config& BufferSize(size_t bufferSize)
			{ STINGRAYKIT_CHECK(bufferSize != 0, "Buffer size can't be zero"); _bufferSize = bufferSize; return *this; }

			size_t BufferSize() const
			{ return _bufferSize; }

			Config& EnableNonBlockingSync()
			{ _nonBlockingSync = true; return *this; }

			bool NonBlockingSync() const
			{ return _nonBlockingSync; }

			std::string ToString() const
			{ return StringBuilder() % "AsyncByteStream::Config { BufferSize: " % _bufferSize % (_nonBlockingSync ? ", NonBlockingSync" : "") % " }"; }
		};

	private:
		struct StreamOp
		{
			STINGRAYKIT_ENUM_VALUES(NoOp, Seek, Write, Stop, Sync, PopBuffer);
			STINGRAYKIT_DECLARE_ENUM_CLASS(StreamOp);
		};

		class StreamOpData
		{
		public:
			StreamOp				_op;
			u64						_arg;

		private:
			StreamOpData(StreamOp op, u64 arg = 0)
				:	_op(op),
					_arg(arg)
			{ }

		public:
			StreamOp Op() const
			{ return _op; }

			s64 SeekingOffset() const
			{ return (s64)_arg; }

			size_t WriteSize() const
			{ return (size_t)_arg; }

			size_t SyncDone() const
			{ return (size_t)_arg; }

			static StreamOpData Seek(s64 offset)
			{ return StreamOpData(StreamOp::Seek, (u64)offset); }

			static StreamOpData Write(size_t size)
			{ return StreamOpData(StreamOp::Write, size); }

			static StreamOpData Stop()
			{ return StreamOpData(StreamOp::Stop); }

			static StreamOpData Sync(size_t syncIndex)
			{ return StreamOpData(StreamOp::Sync, syncIndex); }

			static StreamOpData PopBuffer()
			{ return StreamOpData(StreamOp::PopBuffer); }
		};

		typedef std::deque<StreamOpData> StreamOpQueue;
		typedef std::deque<BithreadCircularBufferPtr> BuffersQueue;

		static NamedLogger			s_logger;

		const std::string			_name;
		const IByteStreamPtr		_stream;

		atomic<bool>				_wasException;

		u64							_position;
		u64							_length;

		Config						_config;

		BuffersQueue				_buffers;
		StreamOpQueue				_streamOpQueue;
		Mutex						_streamOpQueueMutex;

		size_t						_syncNext;
		atomic<size_t>				_syncDone;

		AsyncProfilerPtr			_profiler;

		ConditionVariable			_condVar;
		ConditionVariable			_syncCondVar;
		ThreadPtr					_thread;

	public:
		AsyncByteStream(const std::string& name, const IByteStreamPtr& stream, const Config& config = Config())
			:	_name(name),
				_stream(stream),
				_wasException(false),
				_position(stream->Tell()),
				_length(0),
				_syncNext(1),
				_syncDone(_syncNext - 1),
				_thread(new Thread(StringBuilder() % "AsyncByteStream(" % _name % ")", bind(&AsyncByteStream::ThreadFunc, this, _1)))
		{
			_stream->Seek(0, SeekMode::End);
			_length = stream->Tell();
			_stream->Seek((s64)_position, SeekMode::Begin);
			Reconfigure(config);
			s_logger.Info() << "Created " << _name << " [0.." << _position << ".." << _length << "]";
		}

		~AsyncByteStream()
		{
			s_logger.Info() << "Destroying " << _name << " [0.." << _position << ".." << _length << "]";
			{
				MutexLock l(_streamOpQueueMutex);
				_streamOpQueue.push_back(StreamOpData::Stop());
				_condVar.Broadcast();
			}
			_thread.reset();
		}

		void Reconfigure(const Config& config)
		{
			s_logger.Info() << "Configuring " << _name << ": " << config;
			STINGRAYKIT_CHECK(!_wasException, StringBuilder() % _name % ": was exception while previous operation");

			MutexLock l(_streamOpQueueMutex);
			if (!_buffers.empty())
			{
				_streamOpQueue.push_back(StreamOpData::PopBuffer());
				_condVar.Broadcast();
			}
			_buffers.push_front(make_shared<BithreadCircularBuffer>(config.BufferSize()));
			_config = config;
		}

		virtual u64 Tell() const
		{
			STINGRAYKIT_CHECK(!_wasException, StringBuilder() % _name % ": was exception while previous operation");
			MutexLock l(_streamOpQueueMutex);
			return _position;
		}

		virtual void Seek(s64 offset, SeekMode mode = SeekMode::Begin)
		{
			STINGRAYKIT_PROFILER(50, _name);
			STINGRAYKIT_CHECK(!_wasException, StringBuilder() % _name % ": was exception while previous operation");

			MutexLock l(_streamOpQueueMutex);
			s64 newPosition;

			switch(mode)
			{
			case SeekMode::Begin:
				newPosition = offset;
				break;

			case SeekMode::Current:
				newPosition = (s64)_position + offset;
				break;

			case SeekMode::End:
				newPosition = (s64)_length + offset;
				break;

			default:
				STINGRAYKIT_THROW(NotImplementedException());
			}

			if ((u64)newPosition != _position)
			{
				STINGRAYKIT_CHECK(newPosition >= 0, IndexOutOfRangeException(newPosition, 0, 0));
				_streamOpQueue.push_back(StreamOpData::Seek(newPosition));
				_position = (u64)newPosition;
				// _length increases only after writing
				_condVar.Broadcast();
			}
		}

		virtual u64 Read(ByteData data, const ICancellationToken& token)
		{ STINGRAYKIT_THROW(NotImplementedException(_name)); }

		virtual u64 Write(ConstByteData data, const ICancellationToken& token)
		{
			STINGRAYKIT_PROFILER(50, _name);
			STINGRAYKIT_CHECK(!_wasException, StringBuilder() % _name % ": was exception while previous operation");

			size_t totalWritten = 0;
			MutexLock l(_streamOpQueueMutex);
			BithreadCircularBuffer::Writer writer = _buffers.front()->Write();

			while ((totalWritten != data.size()) && (writer.size() != 0) && token)
			{
				size_t writeSize = std::min(writer.size(), data.size() - totalWritten);
				{
					MutexUnlock ul(_streamOpQueueMutex);
					::memcpy(writer.data(), data.data() + totalWritten, writeSize);
				}
				totalWritten += writeSize;
				writer.Push(writeSize);
				writer = _buffers.front()->Write();
			}

			if (!_streamOpQueue.empty() && (_streamOpQueue.back().Op() == StreamOp::Write))
			{
				const size_t prevWriteSize = _streamOpQueue.back().WriteSize();
				_streamOpQueue.pop_back();
				_streamOpQueue.push_back(StreamOpData::Write(prevWriteSize + totalWritten));
			}
			else
				_streamOpQueue.push_back(StreamOpData::Write(totalWritten));

			_position += totalWritten;
			_length = std::max(_position, _length);
			_condVar.Broadcast();
			return totalWritten;
		}

		virtual void Sync()
		{
			STINGRAYKIT_CHECK(!_wasException, StringBuilder() % _name % ": was exception while previous operation");

			MutexLock l(_streamOpQueueMutex);
			size_t syncCurrent = _syncNext++;
			_streamOpQueue.push_back(StreamOpData::Sync(syncCurrent));
			_condVar.Broadcast();

			if (_config.NonBlockingSync())
			{
				s_logger.Info() << _name << ": sync scheduled";
				return;
			}

			AsyncProfiler::Session profiler_session((_profiler ? _profiler : (_profiler = make_shared<AsyncProfiler>(StringBuilder() % "AsyncByteStream(" % _name % "):profiler"))), "'Sync'", 1000);
			while (true)
			{
				_syncCondVar.TimedWait(_streamOpQueueMutex, TimeDuration::Second());
				STINGRAYKIT_CHECK(!_wasException, StringBuilder() % _name % ": was exception while previous operation");
				if (((ssize_t)(_syncDone - syncCurrent)) >= 0)
					break;
				else
					s_logger.Info() << _name << ": sync done " << _syncDone << ", current " << syncCurrent;
			}
			s_logger.Info() << _name << ": synced";
		}

	private:
		void ThreadFunc(const ICancellationToken& token)
		{
			try
			{
				optional<StreamOpData> opDataHolder;
				MutexLock l(_streamOpQueueMutex);
				while (true)
				{
					if (_streamOpQueue.empty())
					{
						_condVar.Wait(_streamOpQueueMutex);
						continue;
					}

					if (!opDataHolder)
					{
						opDataHolder = _streamOpQueue.front();
						_streamOpQueue.pop_front();
					}
					const StreamOpData& opData = *opDataHolder;

					switch (opData.Op())
					{
					case StreamOp::Seek:
						{
							MutexUnlock ul(l);
							STINGRAYKIT_PROFILER(1000, "Seek");
							_stream->Seek(opData.SeekingOffset(), SeekMode::Begin);
							opDataHolder.reset();
						}
						break;
					case StreamOp::Write:
						{
							size_t written = 0;
							BithreadCircularBuffer::Reader reader = _buffers.back()->Read();
							{
								MutexUnlock ul(l);
								STINGRAYKIT_PROFILER(1000, "Write");
								written = (size_t)_stream->Write(ConstByteData(reader.GetData(), 0, std::min(opData.WriteSize(), reader.size())), token);
							}
							reader.Pop(written);
							if (written != opData.WriteSize())
								opDataHolder = StreamOpData::Write(opData.WriteSize() - written);
							else
								opDataHolder.reset();
						}
						break;
					case StreamOp::Stop:
						opDataHolder.reset();
						return;
					case StreamOp::Sync:
						if (const ISyncableByteStreamPtr syncableStream = dynamic_caster(_stream))
						{
							MutexUnlock ul(l);
							syncableStream->Sync();
						}
						_syncDone = opData.SyncDone();
						_syncCondVar.Broadcast();
						opDataHolder.reset();
						{
							// wake up _syncCondVar waiters
							MutexUnlock ul(l);
							Thread::Yield();
						}
						break;
					case StreamOp::PopBuffer:
						_buffers.pop_back();
						opDataHolder.reset();
						break;
					default:
						STINGRAYKIT_THROW(NotImplementedException(opData.Op().ToString()));
						break;
					}
				}
			}
			catch (const std::exception& ex)
			{
				s_logger.Error() << _name << ": was exception while operating: " << ex;
				_wasException = true;
			}
			s_logger.Error() << _name << ": should never get here";
		}
	};
	STINGRAYKIT_DECLARE_PTR(AsyncByteStream);

}

#endif
