#ifndef __GS_STINGRAY_TOOLKIT_BINARYDATA_H__
#define __GS_STINGRAY_TOOLKIT_BINARYDATA_H__


#include <stingray/toolkit/IBinaryBuffer.h>


namespace stingray
{

	class BinaryData
	{
		struct DummyBuffer : public virtual IBinaryBuffer
		{
			virtual size_t Read(size_t offset, void* data, size_t count) const { return 0; }
			virtual size_t Write(size_t offset, void* data, size_t count) { return 0; }

			virtual size_t GetLength() const { return 0; }
		};

	private:
		IBinaryBufferPtr	_buffer;
		size_t				_offset;
		size_t				_length;

	public:
		BinaryData()
			: _buffer(new DummyBuffer), _offset(0), _length(0)
		{ }

		explicit BinaryData(const IBinaryBufferPtr& buffer)
			: _buffer(TOOLKIT_REQUIRE_NOT_NULL(buffer)), _offset(0), _length(buffer->GetLength())
		{ }

		BinaryData(const BinaryData& other, size_t offset)
			: _buffer(other._buffer), _offset(other._offset + offset), _length(other._length - offset)
		{ TOOLKIT_INDEX_CHECK(offset < other._length); }

		BinaryData(const BinaryData& other, size_t offset, size_t length)
			: _buffer(other._buffer), _offset(other._offset + offset), _length(length)
		{ TOOLKIT_INDEX_CHECK(offset + length <= other._length); }

		size_t Read(size_t offset, void* data, size_t count) const
		{
			TOOLKIT_INDEX_CHECK(offset < _length);
			return _buffer->Read(_offset + offset, data, std::min(_length - offset, count));
		}

		size_t Write(size_t offset, void* data, size_t count)
		{
			TOOLKIT_INDEX_CHECK(offset < _length);
			return _buffer->Write(_offset + offset, data, std::min(_length - offset, count));
		}

		size_t GetLength() const { return _length; }
	};

}


#endif
