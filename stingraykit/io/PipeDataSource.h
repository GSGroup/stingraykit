#ifndef STINGRAYKIT_IO_PIPEDATASOURCE_H
#define STINGRAYKIT_IO_PIPEDATASOURCE_H

#include <stingraykit/io/IDataSource.h>
#include <stingraykit/io/IPipe.h>

namespace stingray
{

	class PipeDataSource : public virtual IDataSource
	{
		static const size_t DefaultBufferSize = 4096;

	private:
		IPipePtr	_pipe;

		ByteArray	_buffer;

	public:
		explicit PipeDataSource(const IPipePtr& pipe, size_t bufferSize = DefaultBufferSize);

		virtual void Read(IDataConsumer& consumer, const ICancellationToken& token);
	};

}

#endif
