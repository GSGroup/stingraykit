#ifndef STINGRAYKIT_IO_BUFFEREDPIPE_H
#define STINGRAYKIT_IO_BUFFEREDPIPE_H

#include <stingraykit/io/IPipe.h>

namespace stingray
{

	class BufferedPipe : public virtual IPipe
	{
		static const size_t DefaultBufferSize = 4096;

	private:
		IPipePtr	_pipe;

		ByteArray	_buffer;

		size_t		_bufferOffset;
		size_t		_bufferSize;

	public:
		explicit BufferedPipe(const IPipePtr& pipe, size_t bufferSize = DefaultBufferSize);

		virtual size_t Read(ByteData data, const ICancellationToken& token);
		virtual size_t Write(ConstByteData data, const ICancellationToken& token)
		{ return _pipe->Write(data, token); }
	};

}

#endif
