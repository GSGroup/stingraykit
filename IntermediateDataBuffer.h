#ifndef STINGRAY_TOOLKIT_INTERMEDIATEDATABUFFER_H
#define STINGRAY_TOOLKIT_INTERMEDIATEDATABUFFER_H


#include <stingray/threads/Thread.h>
#include <stingray/toolkit/BithreadCircularBuffer.h>
#include <stingray/toolkit/IDataSource.h>

namespace stingray
{

	class IntermediateDataBuffer : public virtual IDataSource
	{
	private:
		static NamedLogger		s_logger;
		IDataSourcePtr			_source;
		ThreadPtr				_worker;

		CancellationToken		_token;

		BithreadCircularBuffer	_buffer;
		Mutex					_mutex;
		bool					_eod;

		WaitToken				_bufferEmpty;
		WaitToken				_bufferFull;
		WaitToken				_eodReached;

	public:
		IntermediateDataBuffer(const IDataSourcePtr& source, size_t size) :
			_source(source),
			_buffer(size),
			_eod(false)
		{
			_worker.reset(new Thread("intermediateDataBuffer", bind(&IntermediateDataBuffer::ThreadFunc, this)));
		}

		~IntermediateDataBuffer()
		{
			_token.Cancel();
			_worker->Join();
		}

		virtual void Read(IDataConsumer& consumer, const CancellationToken& token)
		{
			MutexLock l(_mutex);
			BithreadCircularBuffer::Reader r = _buffer.Read();
			if (r.size() == 0)
			{
				if (_eod)
				{
					consumer.EndOfData();
					return;
				}

				_bufferEmpty.Wait(_mutex, token);
				return;
			}

			size_t processed_size = 0;
			{
				MutexUnlock ul(l);
				processed_size = consumer.Process(r.GetData());
			}
			if (processed_size == 0)
				return;
			r.Pop(processed_size);
			_bufferFull.Broadcast();
		}

		void WaitBufferLevel(size_t level, const CancellationToken& token)
		{
			MutexLock l(_mutex);
			while (_buffer.GetDataSize() < level)
				_bufferEmpty.Wait(_mutex, token);
		}

	private:
		size_t DoPush(optional<ConstByteData> data)
		{
			while (_token)
			{
				MutexLock l(_mutex);
				if (!data)
				{
					_eod = true;
					return 0;
				}
				BithreadCircularBuffer::Writer w = _buffer.Write();
				if (w.size() == 0)
				{
					_bufferFull.Wait(_mutex, _token);
					continue;
				}

				const size_t ReadSizeLimit = 0x4000;
				size_t write_size = std::min(data->size(), w.size());
				{
					MutexUnlock ul(l);
					std::copy(data->begin(), data->begin() + write_size, w.begin());
				}
				w.Push(write_size);
				_bufferEmpty.Broadcast();
				return write_size;
			}
			return 0;
		}


		void ThreadFunc()
		{
			while (_token)
			{
				MutexLock l(_mutex);
				if (_eod)
				{
					_eodReached.Wait(_mutex, _token);
					continue;
				}

				MutexUnlock ul(l);
				_source->ReadToFunction(bind(&IntermediateDataBuffer::DoPush, this, _1), _token);
			}
		}
	};
	TOOLKIT_DECLARE_PTR(IntermediateDataBuffer);

}

#endif
