#ifndef STINGRAYKIT_IO_ASYNCBYTESTREAM_H
#define STINGRAYKIT_IO_ASYNCBYTESTREAM_H

// Copyright (c) 2011 - 2015, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/diagnostics/Profiler.h>
#include <stingraykit/function/bind.h>
#include <stingraykit/io/BithreadCircularBuffer.h>
#include <stingraykit/io/IByteStream.h>
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

	class AsyncByteStream : public IByteStream
	{
	private:
		struct StreamOp
		{
			STINGRAYKIT_ENUM_VALUES(NoOp, Seek, Write, Stop);
			STINGRAYKIT_DECLARE_ENUM_CLASS(StreamOp);
		};

		class StreamOpData
		{
		public:
			const StreamOp			Op;
			const u64				SeekingOffset;
			const size_t			WriteSize;

		private:
			StreamOpData(StreamOp op, u64 seekingOffset, size_t writeSize)
				:	Op(op),
					SeekingOffset(seekingOffset),
					WriteSize(writeSize)
			{ }

		public:
			static StreamOpData Seek(u64 offset)
			{ return StreamOpData(StreamOp::Seek, offset, 0); }

			static StreamOpData Write(size_t size)
			{ return StreamOpData(StreamOp::Write, 0, size); }

			static StreamOpData Stop()
			{ return StreamOpData(StreamOp::Stop, 0, 0); }
		};

		typedef std::deque<StreamOpData> StreamOpQueue;

		static NamedLogger			s_logger;

		const std::string			_name;
		const IByteStreamPtr		_stream;

		atomic<bool>				_wasException;

		u64							_position;
		u64							_length;

		BithreadCircularBuffer		_buffer;
		StreamOpQueue				_streamOpQueue;
		Mutex						_streamOpQueueMutex;

		ConditionVariable			_condVar;
		ThreadPtr					_thread;

	public:
		static const size_t DefaultBufferSize = 1 * 1024 * 1024;

		AsyncByteStream(const std::string& name, const IByteStreamPtr& stream, size_t bufferSize = DefaultBufferSize)
			:	_name(name),
				_stream(stream),
				_wasException(false),
				_position(stream->Tell()),
				_length(0),
				_buffer(bufferSize),
				_thread(new Thread(StringBuilder() % "AsyncByteStream(" % name % ")", bind(&AsyncByteStream::ThreadFunc, this, _1)))
		{
			_stream->Seek(0, SeekMode::End);
			_length = stream->Tell();
			_stream->Seek((s64)_position, SeekMode::Begin);
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
				_streamOpQueue.push_back(StreamOpData::Seek((u64)newPosition));
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
			BithreadCircularBuffer::Writer writer = _buffer.Write();

			while ((totalWritten != data.size()) && (writer.size() != 0) && token)
			{
				size_t writeSize = std::min(writer.size(), data.size() - totalWritten);
				{
					MutexUnlock ul(_streamOpQueueMutex);
					::memcpy(writer.data(), data.data() + totalWritten, writeSize);
				}
				totalWritten += writeSize;
				writer.Push(writeSize);
				writer = _buffer.Write();
			}

			_streamOpQueue.push_back(StreamOpData::Write(totalWritten));
			_position += totalWritten;
			_length = std::max(_position, _length);
			_condVar.Broadcast();
			return totalWritten;
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
						opDataHolder.emplace(_streamOpQueue.front());
						_streamOpQueue.pop_front();
					}
					const StreamOpData& opData = *opDataHolder;

					switch (opData.Op)
					{
					case StreamOp::Seek:
						{
							MutexUnlock ul(l);
							STINGRAYKIT_PROFILER(1000, "Seek");
							_stream->Seek((s64)opData.SeekingOffset, SeekMode::Begin);
							opDataHolder.reset();
						}
						break;
					case StreamOp::Write:
						{
							size_t written = 0;
							BithreadCircularBuffer::Reader reader = _buffer.Read();
							{
								MutexUnlock ul(l);
								STINGRAYKIT_PROFILER(1000, "Write");
								written = (size_t)_stream->Write(ConstByteData(reader.GetData(), 0, std::min(opData.WriteSize, reader.size())), token);
							}
							reader.Pop(written);
							if (written != opData.WriteSize)
								opDataHolder.emplace(StreamOpData::Write(opData.WriteSize - written));
							else
								opDataHolder.reset();
						}
						break;
					case StreamOp::Stop:
						opDataHolder.reset();
						return;
					default:
						STINGRAYKIT_THROW(NotImplementedException(opData.Op.ToString()));
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
