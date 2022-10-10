// Copyright (c) 2011 - 2022, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/io/AsyncByteStream.h>

#include <stingraykit/diagnostics/Profiler.h>
#include <stingraykit/function/bind.h>
#include <stingraykit/thread/TimedCancellationToken.h>

#include <string.h>

namespace stingray
{

	namespace
	{

		const size_t DefaultBufferSize = 1 * 1024 * 1024;
		const size_t DefaultPageSize = 4 * 1024;

		const u64 StatsDumpPeriod = 64 * 1024 * 1024;

	}


	AsyncByteStream::Config::Config()
		:	_bufferSize(DefaultBufferSize),
			_pageSize(DefaultPageSize),
			_mergeablePagesHint(16),
			_subStreamsHint(8),
			_nonBlockingSync(false)
	{ }


	AsyncByteStream::Config& AsyncByteStream::Config::BufferSize(size_t bufferSize)
	{
		STINGRAYKIT_CHECK((bufferSize / (2 * _subStreamsHint)) >= (_pageSize * _mergeablePagesHint), "Buffer size is insufficient to work!");
		_bufferSize = bufferSize;
		return *this;
	}


	AsyncByteStream::Config& AsyncByteStream::Config::PageSize(size_t pageSize)
	{
		STINGRAYKIT_CHECK(pageSize != 0, "Page size can't be zero!");
		STINGRAYKIT_CHECK((_bufferSize / (2 * _subStreamsHint)) >= (pageSize * _mergeablePagesHint), "Buffer size is insufficient to work!");
		_pageSize = pageSize;
		return *this;
	}


	AsyncByteStream::Config& AsyncByteStream::Config::MergeablePagesHint(size_t mergeablePagesHint)
	{
		STINGRAYKIT_CHECK(mergeablePagesHint != 0, "Hint number of mergeable pages can't be zero!");
		STINGRAYKIT_CHECK((_bufferSize / (2 * _subStreamsHint)) >= (_pageSize * mergeablePagesHint), "Buffer size is insufficient to work!");
		_mergeablePagesHint = mergeablePagesHint;
		return *this;
	}


	AsyncByteStream::Config& AsyncByteStream::Config::SubStreamsHint(size_t subStreamsHint)
	{
		STINGRAYKIT_CHECK(subStreamsHint != 0, "Hint number of substreams can't be zero!");
		STINGRAYKIT_CHECK((_bufferSize / (2 * subStreamsHint)) >= (_pageSize * _mergeablePagesHint), "Buffer size is insufficient to work!");
		_subStreamsHint = subStreamsHint;
		return *this;
	}


	AsyncByteStream::Config& AsyncByteStream::Config::EnableNonBlockingSync()
	{ _nonBlockingSync = true; return *this; }


	std::string AsyncByteStream::Config::ToString() const
	{ return StringBuilder() % "AsyncByteStream::Config { BufferSize: " % _bufferSize % ", PageSize: " % _pageSize % ", MergeablePagesHint: " % _mergeablePagesHint % ", SubStreamsHint: " % _subStreamsHint % (_nonBlockingSync ? ", NonBlockingSync" : "") % " }"; }


	AsyncByteStream::StreamOpData::StreamOpData(StreamOp op, u64 arg, ByteData buffer)
		:	_op(op),
			_arg(arg),
			_buffer(buffer),
			_used(0)
	{ }


	void AsyncByteStream::StreamOpData::PopWriteData(size_t size)
	{
		_buffer = ByteData(_buffer, size);
		_arg += size;
		_used -= size;
	}


	size_t AsyncByteStream::StreamOpData::PushWriteData(ConstByteData data)
	{
		const size_t appendableSize = std::min(GetWriteFreeSpace(), data.size());
		::memcpy(ByteData(_buffer, _used, appendableSize).data(), data.data(), appendableSize);
		_used += appendableSize;
		return appendableSize;
	}


	AsyncByteStream::Stats::Stats()
		:	Failed(), Appended(), NotAppended(),
			FoundForMerge(), FoundButFull(), FoundButIntersects(), OpQueueLengthSum(), SearchDepthSum(),
			NonFully(), NotSignaled(), PushBufUsage(),
			Syscalls(), TotalWritten()
	{ }


	std::string AsyncByteStream::Stats::ToString() const
	{
		const u64 totalWrites = Failed + Appended + NotAppended;
		const u64 mergeSearches = Appended + NotAppended;
		return StringBuilder() % totalWrites % " write calls (" % Appended % " merged, " % NotAppended % " not, " % NonFully % " partial, " % NotSignaled % " not signaled), "
							   % Failed % " failed, avg. " % ((totalWrites != 0) ? (TotalWritten / totalWrites) : 0) % ", avg. buf " % ((totalWrites != 0) ? (PushBufUsage / totalWrites) : 0) % ", "
							   % FoundForMerge % " found for merge (" % FoundButIntersects % " intersects), " % FoundButFull % " was full, "
							   % "avg. queue length " % ((mergeSearches != 0) ? (OpQueueLengthSum / mergeSearches) : 0) % ", avg. search depth " % ((mergeSearches != 0) ? (SearchDepthSum / mergeSearches) : 0) % ", "
							   % Syscalls % " sys_write calls (totally written " % TotalWritten % ", avg. " % ((Syscalls != 0) ? (TotalWritten / Syscalls) : 0) % ")";
	}


	STINGRAYKIT_DEFINE_NAMED_LOGGER(AsyncByteStream);

	AsyncByteStream::AsyncByteStream(const std::string& name, const IByteStreamPtr& stream, const Config& config)
		:	_name(name),
			_stream(stream),
			_wasException(false),
			_position(stream->Tell()),
			_length(0),
			_bufferPreallocationSize(_config.PageSize() * _config.MergeablePagesHint()),
			_bufferLowWater(0),
			_lastStatsDump(0),
			_syncNext(1),
			_syncDone(_syncNext - 1),
			_thread(make_shared_ptr<Thread>(StringBuilder() % "asyncByteStream(" % _name % ")", Bind(&AsyncByteStream::ThreadFunc, this, _1)))
	{
		_stream->Seek(0, SeekMode::End);
		_length = stream->Tell();
		_stream->Seek((s64)_position, SeekMode::Begin);
		Reconfigure(config);
		s_logger.Info() << "Created " << _name << " [0.." << _position << ".." << _length << "]";
	}


	AsyncByteStream::~AsyncByteStream()
	{
		s_logger.Info() << "Destroying " << _name << " [0.." << _position << ".." << _length << "]";
		{
			MutexLock l(_streamOpQueueMutex);
			_streamOpQueue.push_back(StreamOpData::Stop());
			_condVar.Broadcast();
		}
		_thread.reset();
		s_logger.Info() << "Stats: " << _stats;
	}


	u64 AsyncByteStream::Write(ConstByteData data, const ICancellationToken& token)
	{
		STINGRAYKIT_PROFILER(50, _name);
		STINGRAYKIT_CHECK(!_wasException, StringBuilder() % _name % ": was exception while previous operation");
		if (data.size() == 0)
			return 0;

		MutexLock l(_streamOpQueueMutex);

		STINGRAYKIT_CHECK(!_buffers.empty(), LogicException(StringBuilder() % _name % ": must be at least one buffer"));
		if (_buffers.front()->GetFreeSize() == 0)
		{
			_stats.Failed++;
			_stats.PushBufUsage += _buffers.front()->GetDataSize();
			return 0;
		}

		size_t depth = 0;
		StreamOpQueue::reverse_iterator targetOpDataIt = _streamOpQueue.rend();
		for (StreamOpQueue::reverse_iterator ritopq = _streamOpQueue.rbegin(); ritopq != _streamOpQueue.rend(); ++ritopq, ++depth)
		{
			const StreamOpData& opData = *ritopq;
			if (opData.GetWriteEndOffset() == _position)
			{
				if (opData.GetWriteFreeSpace() > 0)
				{
					targetOpDataIt = ritopq;
					_stats.FoundForMerge++;
				}
				else
					_stats.FoundButFull++;

				break;
			}
		}
		_stats.OpQueueLengthSum += _streamOpQueue.size();
		_stats.SearchDepthSum += depth;

		if (targetOpDataIt != _streamOpQueue.rend())
		{
			const StreamOpData& opData = *targetOpDataIt;
			for (StreamOpQueue::iterator itopq = targetOpDataIt.base(); itopq != _streamOpQueue.end(); ++itopq)
			{
				if (opData.IsWriteIntersects(*itopq))
				{
					targetOpDataIt = _streamOpQueue.rend();
					_stats.FoundButIntersects++;
					break;
				}
			}
		}

		if (targetOpDataIt == _streamOpQueue.rend())
		{
			BithreadCircularBuffer::Writer writer = _buffers.front()->Write();
			size_t writeSize = std::min(writer.size(), std::max(_bufferPreallocationSize, data.size()));

			ByteData writeData(writer.GetData(), 0, writeSize);
			writer.Push(writeSize);

			_streamOpQueue.push_back(StreamOpData::Write(_position, writeData));
			targetOpDataIt = _streamOpQueue.rbegin();
			_stats.NotAppended++;
		}
		else
			_stats.Appended++;

		StreamOpData& opData = *targetOpDataIt;
		size_t written = opData.PushWriteData(data);

		if (written != data.size())
			_stats.NonFully++;

		const size_t bufUsed = _buffers.front()->GetDataSize();
		_stats.PushBufUsage += bufUsed;

		_position += written;
		_length = std::max(_position, _length);

		if (bufUsed >= _bufferLowWater)
			_condVar.Broadcast();
		else
			_stats.NotSignaled++;

		return written;
	}


	void AsyncByteStream::Seek(s64 offset, SeekMode mode)
	{
		STINGRAYKIT_PROFILER(50, _name);
		STINGRAYKIT_CHECK(!_wasException, StringBuilder() % _name % ": was exception while previous operation");

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
			_position = (u64)newPosition;
			// _length increases only after writing
		}
	}


	u64 AsyncByteStream::Tell() const
	{
		STINGRAYKIT_CHECK(!_wasException, StringBuilder() % _name % ": was exception while previous operation");
		return _position;
	}


	void AsyncByteStream::Sync()
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

		AsyncProfiler::Session profiler_session((_profiler ? _profiler : (_profiler = make_shared_ptr<AsyncProfiler>(StringBuilder() % "AsyncByteStream(" % _name % "):profiler"))), "'Sync'", TimeDuration::Second());
		while (true)
		{
			_syncCondVar.Wait(_streamOpQueueMutex, TimedCancellationToken(TimeDuration::Second()));
			STINGRAYKIT_CHECK(!_wasException, StringBuilder() % _name % ": was exception while previous operation");
			if (((ssize_t)(_syncDone - syncCurrent)) >= 0)
				break;
			else
				s_logger.Info() << _name << ": sync done " << _syncDone << ", current " << syncCurrent;
		}
		s_logger.Info() << _name << ": synced";
	}


	void AsyncByteStream::Reconfigure(const Config& config)
	{
		s_logger.Info() << "Configuring " << _name << ": " << config;
		STINGRAYKIT_CHECK(!_wasException, StringBuilder() % _name % ": was exception while previous operation");

		MutexLock l(_streamOpQueueMutex);
		if (!_buffers.empty())
		{
			_streamOpQueue.push_back(StreamOpData::PopBuffer());
			_condVar.Broadcast();
		}
		_buffers.push_front(make_shared_ptr<BithreadCircularBuffer>(config.BufferSize()));
		_config = config;
		_bufferPreallocationSize = _config.PageSize() * _config.MergeablePagesHint();
		_bufferLowWater = std::max(_config.BufferSize() / 10, std::min(_config.BufferSize() / 2, _bufferPreallocationSize * 2));
	}


	void AsyncByteStream::ThreadFunc(const ICancellationToken& token)
	{
		try
		{
			bool active = true;
			MutexLock l(_streamOpQueueMutex);
			while (active)
			{
				if (_streamOpQueue.empty())
				{
					if (token)
						_condVar.Wait(_streamOpQueueMutex, TimedCancellationToken(token, TimeDuration::Second()));
					else
						STINGRAYKIT_ASSUME_CANCELLATION(token);

					continue;
				}

				StreamOpData opData = _streamOpQueue.front();
				_streamOpQueue.pop_front();

				switch (opData.Op())
				{
				case StreamOp::Write:
					{
						size_t written = 0;

						STINGRAYKIT_CHECK(!_buffers.empty(), LogicException(StringBuilder() % _name % ": must be at least one buffer"));
						BithreadCircularBuffer::Reader reader = _buffers.back()->Read();

						ConstByteData writeData = opData.GetWriteData();
						STINGRAYKIT_CHECK(writeData.data() == reader.GetData().data(), LogicException(StringBuilder() % _name % ": buffer pointers out of sync: " % writeData.data() % " != " % reader.GetData().data()));

						{
							MutexUnlock ul(l);
							STINGRAYKIT_PROFILER(1000, "Write");
							_stream->Seek((s64)opData.GetWriteStartOffset(), SeekMode::Begin);
							written = (size_t)_stream->Write(writeData, token);
						}

						if (written == writeData.size())
							reader.Pop(written + opData.GetWriteFreeSpace());
						else
						{
							reader.Pop(written);
							opData.PopWriteData(written);
							_streamOpQueue.push_front(opData);
						}

						_stats.Syscalls++;
						_stats.TotalWritten += written;
					}
					break;

				case StreamOp::Stop:
					active = false;
					break;

				case StreamOp::Sync:
					if (const ISyncableByteStreamPtr syncableStream = dynamic_caster(_stream))
					{
						MutexUnlock ul(l);
						syncableStream->Sync();
					}
					_syncDone = opData.SyncDone();
					_syncCondVar.Broadcast();
					{
						// wake up _syncCondVar waiters
						MutexUnlock ul(l);
						Thread::Yield();
					}
					break;

				case StreamOp::PopBuffer:
					_buffers.pop_back();
					break;

				default:
					STINGRAYKIT_THROW(NotImplementedException(opData.Op().ToString()));
					break;
				}

				if ((_stats.TotalWritten - _lastStatsDump) >= StatsDumpPeriod)
				{
					s_logger.Info() << _name << " stats: " << _stats;
					_lastStatsDump = _stats.TotalWritten;
				}
			}
		}
		catch (const std::exception& ex)
		{
			s_logger.Error() << _name << ": was exception while operating: " << ex;
			_wasException = true;
		}
	}

}
