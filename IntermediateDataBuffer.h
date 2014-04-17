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
		IntermediateDataBuffer(const std::string& threadName, const IDataSourcePtr& source, size_t size, size_t inputQuantSize = 1, size_t outputQuantSize = 1) :
			_source(source), _buffer(size, inputQuantSize, outputQuantSize)
		{
			_worker.reset(new Thread(threadName, bind(&IntermediateDataBuffer::ThreadFunc, this)));
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
