#ifndef STINGRAY_TOOLKIT_IO_INTERMEDIATEDATABUFFER_H
#define STINGRAY_TOOLKIT_IO_INTERMEDIATEDATABUFFER_H


#include <stingray/threads/Thread.h>
#include <stingray/toolkit/io/IDataSource.h>
#include <stingray/toolkit/io/DataBuffer.h>

namespace stingray
{

	class IntermediateDataBuffer : public virtual IDataSource
	{
	private:
		static NamedLogger		s_logger;
		IDataSourcePtr			_source;
		IDataBufferPtr			_buffer;

		ThreadPtr				_worker;

	public:
		IntermediateDataBuffer(const std::string& threadName, const IDataSourcePtr& source, const IDataBufferPtr& buffer) :
			_source(source), _buffer(buffer)
		{
			_worker.reset(new Thread(threadName, bind(&IntermediateDataBuffer::ThreadFunc, this, _1)));
		}

		~IntermediateDataBuffer()
		{ _worker.reset(); }

		virtual void Read(IDataConsumer& consumer, const ICancellationToken& token)
		{ return _buffer->Read(consumer, token); }

	private:
		void ThreadFunc(const ICancellationToken& token)
		{
			while (token)
				_source->Read(*_buffer, token);
		}
	};
	TOOLKIT_DECLARE_PTR(IntermediateDataBuffer);

}

#endif
