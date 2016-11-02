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
#include <stingraykit/io/IByteStream.h>
#include <stingraykit/log/Logger.h>
#include <stingraykit/thread/ConditionVariable.h>
#include <stingraykit/thread/Thread.h>
#include <stingraykit/thread/atomic.h>
#include <stingraykit/toolkit.h>

#include <queue>

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

			const SeekMode			SeekingMode;
			const s64				SeekingOffset;

			const ConstByteArray	WriteData;

		private:
			StreamOpData(StreamOp op, s64 seekingOffset, SeekMode seekingMode, const ConstByteArray& writeData)
				:	Op(op),
					SeekingMode(seekingMode),
					SeekingOffset(seekingOffset),
					WriteData(writeData)
			{ }

		public:
			StreamOpData()
				:	Op(StreamOp::NoOp),
					SeekingMode(SeekMode()),
					SeekingOffset(0),
					WriteData(null)
			{ }

			static StreamOpData Seek(s64 offset, SeekMode mode)
			{ return StreamOpData(StreamOp::Seek, offset, mode, ConstByteArray(null)); }

			static StreamOpData Write(ConstByteData data)
			{ return StreamOpData(StreamOp::Write, 0, SeekMode(), ConstByteArray(data)); }

			static StreamOpData Stop()
			{ return StreamOpData(StreamOp::Stop, 0, SeekMode(), ConstByteArray(null)); }
		};

		typedef std::queue<StreamOpData> StreamOpQueue;

		static NamedLogger			s_logger;

		const std::string			_name;
		const IByteStreamPtr		_stream;

		atomic<bool>				_wasException;

		StreamOpQueue				_streamOpQueue;
		Mutex						_streamOpQueueMutex;

		ConditionVariable			_condVar;
		ThreadPtr					_thread;

	public:
		AsyncByteStream(const std::string& name, const IByteStreamPtr& stream)
			:	_name(name),
				_stream(stream),
				_wasException(false),
				_thread(new Thread(StringBuilder() % "AsyncByteStream(" % name % ")", bind(&AsyncByteStream::ThreadFunc, this, _1)))
		{ s_logger.Info() << "Created " << _name; }

		~AsyncByteStream()
		{
			s_logger.Info() << "Destroying " << _name;
			{
				MutexLock l(_streamOpQueueMutex);
				_streamOpQueue.push(StreamOpData::Stop());
				_condVar.Broadcast();
			}
			_thread.reset();
		}

		virtual u64 Tell() const
		{ STINGRAYKIT_THROW(NotImplementedException(_name)); }

		virtual void Seek(s64 offset, SeekMode mode = SeekMode::Begin)
		{
			STINGRAYKIT_PROFILER(50, _name);
			STINGRAYKIT_CHECK(!_wasException, StringBuilder() % _name % ": was exception while previous operation");
			MutexLock l(_streamOpQueueMutex);
			_streamOpQueue.push(StreamOpData::Seek(offset, mode));
			_condVar.Broadcast();
		}

		virtual u64 Read(ByteData data, const ICancellationToken& token)
		{ STINGRAYKIT_THROW(NotImplementedException(_name)); }

		virtual u64 Write(ConstByteData data, const ICancellationToken& token)
		{
			STINGRAYKIT_PROFILER(50, _name);
			STINGRAYKIT_CHECK(!_wasException, StringBuilder() % _name % ": was exception while previous operation");
			MutexLock l(_streamOpQueueMutex);
			_streamOpQueue.push(StreamOpData::Write(data));
			_condVar.Broadcast();
			return data.size();
		}

	private:
		void ThreadFunc(const ICancellationToken& token)
		{
			try
			{
				MutexLock l(_streamOpQueueMutex);
				while (true)
				{
					if (_streamOpQueue.empty())
					{
						_condVar.Wait(_streamOpQueueMutex);
						continue;
					}

					StreamOpData opData = _streamOpQueue.front();
					_streamOpQueue.pop();

					MutexUnlock ul(l);
					switch (opData.Op)
					{
					case StreamOp::Seek:
						{
							STINGRAYKIT_PROFILER(1000, "Seek");
							_stream->Seek(opData.SeekingOffset, opData.SeekingMode);
						}
						break;
					case StreamOp::Write:
						{
							STINGRAYKIT_PROFILER(1000, "Write");
							_stream->Write(opData.WriteData.GetByteData(), token);
						}
						break;
					case StreamOp::Stop:
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
