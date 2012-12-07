#ifndef STINGRAY_TOOLKIT_MEMORYBYTESTREAM_H
#define STINGRAY_TOOLKIT_MEMORYBYTESTREAM_H


#include <stingray/toolkit/IByteStream.h>
#include <stingray/toolkit/ByteData.h>


namespace stingray
{


	class MemoryByteStream : public virtual IByteStream
	{
	private:
		ConstByteData	_data;
		size_t			_ofs;

	public:
		MemoryByteStream(const ConstByteData& data)
			: _data(data), _ofs(0)
		{ }

		virtual size_t Read(void* data, size_t count)
		{
			u8* dst = reinterpret_cast<u8*>(data);
			count = std::min(count, _data.size() - _ofs);
			std::copy(_data.begin() + _ofs, _data.begin() + _ofs + count, dst);
			_ofs += count;
			return count;
		}

		virtual size_t Write(const void* data, size_t count)
		{ TOOLKIT_THROW(NotImplementedException()); }

		virtual void Seek(int offset, SeekMode mode = SeekMode::Begin)
		{
			int new_ofs = _ofs;
			switch (mode)
			{
			case SeekMode::Begin:	new_ofs = offset; break;
			case SeekMode::Current:	new_ofs += offset; break;
			case SeekMode::End:		new_ofs = _data.size() + offset; break;
			}
			if (new_ofs < 0)
				TOOLKIT_THROW(IndexOutOfRangeException());
			if (new_ofs > (int)_data.size())
				TOOLKIT_THROW(NotImplementedException());
			_ofs = new_ofs;
		}

		virtual size_t Tell() const
		{ return _ofs; }
	};


}


#endif
