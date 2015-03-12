#ifndef STINGRAYKIT_IO_PIPEREADER_H
#define STINGRAYKIT_IO_PIPEREADER_H

#include <stingraykit/io/IPipe.h>

namespace stingray
{

	class PipeReader
	{
	private:
		IPipePtr				_pipe;

	public:
		explicit PipeReader(const IPipePtr& pipe);

		size_t Read(ByteData data, const ICancellationToken& token)
		{ return _pipe->Read(data, token); }

		u8 ReadByte(const ICancellationToken& token);

		/** Only \n and \r\n line endings are supported */
		std::string ReadLine(const ICancellationToken& token);
	};
	STINGRAYKIT_DECLARE_PTR(PipeReader);

}

#endif
