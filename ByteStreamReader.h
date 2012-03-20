#ifndef __GS_STINGRAY_TOOLKIT_BYTESTREAMREADER_H__
#define __GS_STINGRAY_TOOLKIT_BYTESTREAMREADER_H__


#include <string>

#include <stingray/toolkit/BitsGetter.h>
#include <stingray/toolkit/IByteStream.h>


namespace stingray
{


	class ByteStreamReader
	{
	private:
		IByteStreamPtr	_stream;

	public:
		explicit ByteStreamReader(const IByteStreamPtr& stream)
			: _stream(TOOLKIT_REQUIRE_NOT_NULL(stream))
		{ TOOLKIT_CHECK(_stream->CanRead(), ArgumentException("stream")); }

		template < typename PodType >
		PodType Read()
		{
			u8 buffer[sizeof(PodType)];
			TOOLKIT_CHECK(_stream->Read(buffer, sizeof(PodType)) == sizeof(PodType), EndOfStreamException());
			return BitsGetter(ConstByteData(buffer, sizeof(PodType))).Get<PodType>();
		}

		std::string ReadString(char terminator)
		{
			std::string result;
			for (char ch = Read<u8>(); ch != terminator; ch = Read<u8>())
				result.push_back(ch);
			return result;
		}

		std::string ReadNullTerminatedString()
		{ return ReadString('\0'); }
	};

}


#endif
