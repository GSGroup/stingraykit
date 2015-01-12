#ifndef STINGRAY_TOOLKIT_IO_CHUNKEDMEMORYBYTESTREAM_H
#define STINGRAY_TOOLKIT_IO_CHUNKEDMEMORYBYTESTREAM_H

#include <stingray/toolkit/collection/ByteData.h>
#include <stingray/toolkit/io/IByteStream.h>


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
		size_t								_totalSize;

	public:
		ChunkedMemoryByteStream(const ContainerTypePtr & container)
			: _container(container), _chunk(_container->begin()), _chunkOffset(0), _totalSize(TotalSize())
		{ }

		virtual u64 Read(ByteData data)
		{
			STINGRAYKIT_CHECK(!_container->empty(), LogicException("reading from empty container"));
			NormalizePosition();
			STINGRAYKIT_CHECK(_chunk != _container->end(), LogicException("reading from end of chunks"));

			size_t passed = 0;
			for (size_t curcount = 0; _chunk != _container->end() && passed < data.size(); passed += curcount)
			{
				curcount = std::min(data.size() - passed, _chunk->size() - _chunkOffset);
				std::copy(_chunk->begin() + _chunkOffset, _chunk->begin() + _chunkOffset + curcount, data.data() + passed);
				if (_chunkOffset + curcount == _chunk->size())
				{
					++_chunk;
					_chunkOffset = 0;
				}
				else
					_chunkOffset += curcount;
			}
			return passed;
		}

		virtual u64 Write(ConstByteData data)
		{
			STINGRAYKIT_CHECK(data.size() > 0, LogicException("writing empty data"));
			NormalizePosition();

			size_t passed = 0;
			for (size_t curcount = 0; passed < data.size(); passed += curcount)
			{
				if (_chunk == _container->end())
				{
					STINGRAYKIT_CHECK(_chunkOffset == 0, LogicException("_chunkOffset must be 0"));
					curcount = data.size() - passed;
					_container->push_back(ChunkType(data.data() + passed, curcount));
					_chunk = _container->end();
					_chunkOffset = 0;
					_totalSize = TotalSize();
				}
				else
				{
					STINGRAYKIT_CHECK(_chunk->size() > _chunkOffset, IndexOutOfRangeException(_chunkOffset, _chunk->size()));
					curcount = std::min(data.size() - passed, _chunk->size() - _chunkOffset);
					std::copy(data.data() + passed, data.data() + passed + curcount, _chunk->begin() + _chunkOffset);
					_chunkOffset += curcount;
				}
			}
			return passed;
		}

		virtual void Seek(s64 offset, SeekMode mode = SeekMode::Begin)
		{
			STINGRAYKIT_CHECK(!_container->empty(), LogicException("seeking within empty container"));

			switch (mode)
			{
			case SeekMode::Begin:	break;
			case SeekMode::Current:	offset += static_cast<s64>(Tell()); break;
			case SeekMode::End:		offset += static_cast<s64>(_totalSize); break;
			default:				STINGRAYKIT_THROW(ArgumentException("mode")); break;
			}

			STINGRAYKIT_CHECK(offset >= 0 && (u64)offset <= _totalSize, IndexOutOfRangeException(offset, _totalSize));
			size_t offset_ = offset;

			typename ContainerType::iterator chunk;
			for (chunk = _container->begin(); chunk != _container->end() && offset_ >= chunk->size(); ++chunk)
				offset_ -= chunk->size();

			_chunk = chunk;
			_chunkOffset = offset_;
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

		void NormalizePosition()
		{
			if (_chunk != _container->end() && _chunkOffset == _chunk->size())
			{
				_chunk++;
				_chunkOffset = 0;
			}
		}
	};

}


#endif
