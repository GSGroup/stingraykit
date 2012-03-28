#ifndef __GS_STINGRAY_TOOLKIT_MEMORYBINARYBUFFER_H__
#define __GS_STINGRAY_TOOLKIT_MEMORYBINARYBUFFER_H__


#include <stingray/toolkit/BinaryData.h>


namespace stingray
{

	template < typename ByteDataType >
	class MemoryBinaryBuffer : public virtual IBinaryBuffer
	{
	private:
		ByteDataType	_data;

	public:
		explicit MemoryBinaryBuffer(const ByteDataType& data)
			: _data(data)
		{ }

		virtual size_t Read(size_t offset, void* data, size_t count) const
		{
			TOOLKIT_INDEX_CHECK(offset <= _data.size());
			const size_t result = std::min(_data.size() - offset, count);
			std::copy(_data.begin() + offset, _data.begin() + offset + result, (u8*)data);
			return result;
		}

		virtual size_t Write(size_t offset, void* data, size_t count)
		{ TOOLKIT_NOT_IMPLEMENTED(); }

		virtual size_t GetLength() const { return _data.size(); }
	};

}


#endif
