#ifndef __GS_STINGRAY_TOOLKIT_BINARYSTREAM_H__
#define __GS_STINGRAY_TOOLKIT_BINARYSTREAM_H__


#include <stingray/toolkit/BinaryData.h>
#include <stingray/toolkit/BitsGetter.h>


namespace stingray
{

	TOOLKIT_DECLARE_SIMPLE_EXCEPTION(EndOfStreamException, "End of stream reached!");


	class BinaryStream
	{
	private:
		BinaryData	_buffer;
		size_t		_offset;

	public:
		explicit BinaryStream(const BinaryData& buffer) : _buffer(buffer), _offset(0) { }
		BinaryStream(const BinaryData& buffer, size_t offset) : _buffer(buffer, offset), _offset(0) { }
		BinaryStream(const BinaryData& buffer, size_t offset, size_t size) : _buffer(buffer, offset, size), _offset(0) { }
		BinaryStream(const BinaryStream& other, size_t offset) : _buffer(other._buffer, offset), _offset(0) { }
		BinaryStream(const BinaryStream& other, size_t offset, size_t size) : _buffer(other._buffer, offset, size), _offset(0) { }

		template < typename PodType >
		PodType Read()
		{
			u8 buffer[sizeof(PodType)];
			TOOLKIT_CHECK(_buffer.Read(_offset, buffer, sizeof(PodType)) == sizeof(PodType), EndOfStreamException());
			_offset += sizeof(PodType);
			return BitsGetter(ByteData(buffer, sizeof(PodType))).Get<PodType>();
		}

		std::string ReadStringUntil(char delimiter)
		{
			std::string result;
			for (char ch = Read<u8>(); ch != delimiter; ch = Read<u8>())
				result.push_back(ch);
			return result;
		}

		std::string ReadNullTerminatedString()
		{ return ReadStringUntil('\0'); }

		void Seek(size_t offset)
		{
			TOOLKIT_INDEX_CHECK(offset <= _buffer.GetLength());
			_offset = offset;
		}

		void Skip(size_t count)
		{
			TOOLKIT_INDEX_CHECK(_offset + count <= _buffer.GetLength());
			_offset += count;
		}

		size_t		GetOffset() const { return _offset; }
		BinaryData	GetBuffer() const { return _buffer; }
	};

}


#endif
