#ifndef STINGRAY_TOOLKIT_CHUNKEDMEMORYBYTESTREAM_H
#define STINGRAY_TOOLKIT_CHUNKEDMEMORYBYTESTREAM_H

#include <stingray/toolkit/ByteData.h>
#include <stingray/toolkit/IByteStream.h>


namespace stingray
{

	template<typename ChunkType, typename ContainerType = std::vector<ChunkType> >
	class ChunkedMemoryByteStream : public IByteStream
	{
	public:
		typedef shared_ptr<ContainerType> ContainerTypePtr;

	private:

		ContainerTypePtr					_container;
		typename ContainerType::iterator	_chunk;
		size_t								_chunkOffset;

	public:
		ChunkedMemoryByteStream(const ContainerTypePtr & container)
			: _container(container), _chunk(_container->begin()), _chunkOffset(0)
		{ }

		virtual u64 Read(ByteData data)
		{
			TOOLKIT_CHECK(!_container->empty(), LogicException("reading from empty container"));
			TOOLKIT_CHECK(_chunk != _container->end(), LogicException("reading from end of chunks"));
			TOOLKIT_CHECK(_chunk->size() > _chunkOffset, IndexOutOfRangeException(_chunkOffset, _chunk->size()));
			size_t count = std::min(data.size(), _chunk->size() - _chunkOffset);
			std::copy(_chunk->begin() + _chunkOffset, _chunk->begin() + _chunkOffset + count, data.data());
			if (_chunkOffset + count == _chunk->size())
			{
				++_chunk;
				_chunkOffset = 0;
			}
			else
				_chunkOffset += count;
			return count;
		}

		virtual u64 Write(ConstByteData data)
		{
			TOOLKIT_CHECK(data.size() > 0, LogicException("writing empty data"));
			if (_container->empty() || _chunk == _container->end() || _chunkOffset == _chunk->size())
			{
				_container->push_back(ChunkType(data.data(), data.size()));
				_chunk = _container->end();
				_chunkOffset = 0;
				return data.size();
			}
			else
			{
				TOOLKIT_CHECK(_chunk->size() > _chunkOffset, IndexOutOfRangeException(_chunkOffset, _chunk->size()));
				size_t count = std::min(data.size(), _chunk->size() - _chunkOffset);
				std::copy(data.data(), data.data() + count, _chunk->begin() + _chunkOffset);
				_chunkOffset += count;
				return count;
			}
		}

		virtual void Seek(s64 offset, SeekMode mode = SeekMode::Begin)
		{
			TOOLKIT_CHECK(!_container->empty(), LogicException("seeking within empty container"));

			size_t total_size = TotalSize();

			switch (mode)
			{
			case SeekMode::Begin:	break;
			case SeekMode::Current:	offset += static_cast<s64>(Tell()); break;
			case SeekMode::End:		offset += static_cast<s64>(total_size); break;
			default:				TOOLKIT_THROW(ArgumentException("mode")); break;
			}

			TOOLKIT_CHECK(offset >= 0 && offset <= total_size, IndexOutOfRangeException(offset, total_size));

			typename ContainerType::iterator chunk;
			for (chunk = _container->begin(); chunk != _container->end() && offset >= chunk->size(); ++chunk)
				offset -= chunk->size();

			_chunk = chunk;
			_chunkOffset = offset;
		}

		virtual u64 Tell() const
		{
			size_t ret = 0;
			for (typename ContainerType::const_iterator chunk = _container->begin(); chunk != _chunk; ++chunk)
				ret += chunk->size();
			ret += _chunkOffset;
			return ret;
		}

	private:
		size_t TotalSize() const
		{
			size_t ret = 0;
			for (typename ContainerType::const_iterator chunk = _container->begin(); chunk != _container->end(); ++chunk)
				ret += chunk->size();
			return ret;
		}
	};

}


#endif
