#ifndef STINGRAYKIT_IO_ASYNCBYTESTREAM_H
#define STINGRAYKIT_IO_ASYNCBYTESTREAM_H

// Copyright (c) 2011 - 2018, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/diagnostics/AsyncProfiler.h>
#include <stingraykit/io/BithreadCircularBuffer.h>
#include <stingraykit/io/ISyncableByteStream.h>
#include <stingraykit/log/Logger.h>
#include <stingraykit/thread/ConditionVariable.h>

#include <deque>

namespace stingray
{

	class AsyncByteStream : public virtual ISyncableByteStream
	{
	public:
		class Config
		{
			size_t					_bufferSize;
			size_t					_pageSize;
			size_t					_mergeablePagesHint;
			size_t					_subStreamsHint;
			bool					_nonBlockingSync;

		public:
			Config();

			Config& BufferSize(size_t bufferSize);
			size_t BufferSize() const
			{ return _bufferSize; }

			Config& PageSize(size_t pageSize);
			size_t PageSize() const
			{ return _pageSize; }

			Config& MergeablePagesHint(size_t mergeablePagesHint);
			size_t MergeablePagesHint() const
			{ return _mergeablePagesHint; }

			Config& SubStreamsHint(size_t subStreamsHint);
			size_t SubStreamsHint() const
			{ return _subStreamsHint; }

			Config& EnableNonBlockingSync();
			bool NonBlockingSync() const
			{ return _nonBlockingSync; }

			std::string ToString() const;
		};

	private:
		struct StreamOp
		{
			STINGRAYKIT_ENUM_VALUES(NoOp, Write, Stop, Sync, PopBuffer);
			STINGRAYKIT_DECLARE_ENUM_CLASS(StreamOp);
		};

		class StreamOpData
		{
		private:
			StreamOp				_op;
			u64						_arg;
			ByteData				_buffer;
			size_t					_used;

			StreamOpData(StreamOp op, u64 arg = 0, ByteData buffer = ByteData());

			u64 GetWriteLimitOffset() const
			{ return _arg + _buffer.size(); }

		public:
			StreamOp Op() const
			{ return _op; }

			u64 GetWriteStartOffset() const
			{ return _arg; }

			u64 GetWriteEndOffset() const
			{ return _arg + _used; }

			bool IsWriteIntersects(const StreamOpData& other) const
			{ return (GetWriteStartOffset() < other.GetWriteLimitOffset()) && (other.GetWriteStartOffset() < GetWriteLimitOffset()); }

			ConstByteData GetWriteData() const
			{ return ConstByteData(_buffer, 0, _used); }

			size_t GetWriteFreeSpace() const
			{ return _buffer.size() - _used; }

			void PopWriteData(size_t size);
			size_t PushWriteData(ConstByteData data);

			size_t SyncDone() const
			{ return (size_t)_arg; }

			static StreamOpData Write(u64 offset, ByteData buffer)
			{ return StreamOpData(StreamOp::Write, offset, buffer); }

			static StreamOpData Stop()
			{ return StreamOpData(StreamOp::Stop); }

			static StreamOpData Sync(size_t syncIndex)
			{ return StreamOpData(StreamOp::Sync, syncIndex); }

			static StreamOpData PopBuffer()
			{ return StreamOpData(StreamOp::PopBuffer); }
		};

		struct Stats
		{
			u64						Failed;
			u64						Appended;
			u64						NotAppended;

			u64						FoundForMerge;
			u64						FoundButFull;
			u64						FoundButIntersects;
			u64						OpQueueLengthSum;
			u64						SearchDepthSum;

			u64						NonFully;
			u64						NotSignaled;
			u64						PushBufUsage;

			u64						Syscalls;
			u64						TotalWritten;

			Stats();

			std::string ToString() const;
		};

		typedef std::deque<StreamOpData> StreamOpQueue;
		typedef std::deque<BithreadCircularBufferPtr> BuffersQueue;

	private:
		static NamedLogger			s_logger;

		const std::string			_name;
		const IByteStreamPtr		_stream;

		atomic<bool>				_wasException;

		u64							_position;
		u64							_length;

		Config						_config;
		size_t						_bufferPreallocationSize;
		size_t						_bufferLowWater;

		BuffersQueue				_buffers;
		StreamOpQueue				_streamOpQueue;
		Mutex						_streamOpQueueMutex;

		Stats						_stats;
		u64							_lastStatsDump;

		size_t						_syncNext;
		atomic<size_t>				_syncDone;

		AsyncProfilerPtr			_profiler;

		ConditionVariable			_condVar;
		ConditionVariable			_syncCondVar;
		ThreadPtr					_thread;

	public:
		AsyncByteStream(const std::string& name, const IByteStreamPtr& stream, const Config& config = Config());
		virtual ~AsyncByteStream();

		virtual u64 Read(ByteData data, const ICancellationToken& token)
		{ STINGRAYKIT_THROW(NotImplementedException(_name)); }

		virtual u64 Write(ConstByteData data, const ICancellationToken& token);

		virtual void Seek(s64 offset, SeekMode mode = SeekMode::Begin);
		virtual u64 Tell() const;

		virtual void Sync();

		void Reconfigure(const Config& config);

	private:
		void ThreadFunc(const ICancellationToken& token);
	};
	STINGRAYKIT_DECLARE_PTR(AsyncByteStream);

}

#endif
