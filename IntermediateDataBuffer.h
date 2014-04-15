#ifndef STINGRAY_TOOLKIT_INTERMEDIATEDATABUFFER_H
#define STINGRAY_TOOLKIT_INTERMEDIATEDATABUFFER_H


#include <stingray/threads/Thread.h>
#include <stingray/toolkit/BithreadCircularBuffer.h>
#include <stingray/toolkit/IDataSource.h>
#include <stingray/toolkit/SimplePacketedBuffer.h>

namespace stingray
{

	class IntermediateDataBuffer : public virtual IDataSource
	{
	private:
		static NamedLogger		s_logger;
		IDataSourcePtr			_source;
		SimplePacketedBuffer	_buffer;

		ThreadPtr				_worker;
		CancellationToken		_token;

	public:
		IntermediateDataBuffer(const IDataSourcePtr& source, size_t size) :
			_source(source),
			_buffer(size, 1, 1)
		{
			_worker.reset(new Thread("intermediateDataBuffer", bind(&IntermediateDataBuffer::ThreadFunc, this)));
		}

		~IntermediateDataBuffer()
		{
			_token.Cancel();
			_worker->Join();
		}

		virtual void Read(IDataConsumer& consumer, const CancellationToken& token)
		{ return _buffer.Read(consumer, token); }

	private:
		void ThreadFunc()
		{
			while (_token)
				_source->Read(_buffer, _token);
		}
	};
	TOOLKIT_DECLARE_PTR(IntermediateDataBuffer);

}

#endif
