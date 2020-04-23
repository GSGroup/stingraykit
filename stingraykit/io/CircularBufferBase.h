#ifndef STINGRAYKIT_IO_CIRCULARBUFFERBASE_H
#define STINGRAYKIT_IO_CIRCULARBUFFERBASE_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <limits>
#include <algorithm>

#include <stingraykit/log/Logger.h>
#include <stingraykit/io/ICircularBuffer.h>
#include <stingraykit/collection/ByteData.h>
#include <stingraykit/function/bind.h>


namespace stingray
{

	/**
	 * @addtogroup core_storage
	 * @{
	 */

	template <bool DiscardOnOverflow = false>
	class CircularBufferBase : public virtual ICircularBuffer
	{
	protected:
		typedef u8 DataType;

		virtual void WriteToStorage(size_t offset, ConstByteData::iterator begin, ConstByteData::iterator end) = 0;
		virtual ConstByteData ReadStorage(size_t offset, size_t size) const = 0;

	private:
		static NamedLogger	s_logger;

		size_t				_writeOffset;
		size_t				_readOffset;
		size_t				_lockedDataSize;
		Mutex				_mutex;
		bool 				_loggingEnabled;

		ConstByteData CheckDataSize(const ConstByteData& data)
		{
			const size_t totalCapacity = (_writeOffset >= _readOffset) ? (GetStorageSize() - _writeOffset + _readOffset - 1)
																  : (_readOffset - _writeOffset - 1);
			if (data.size() > totalCapacity)
			{
				if (DiscardOnOverflow)
				{
					const size_t resultSize = std::min(data.size(), GetStorageSize() - 1);
					ConstByteData croppedData(data, data.size() - resultSize, resultSize);
					Pop(resultSize - totalCapacity);
					return croppedData;
				}

				STINGRAYKIT_THROW(BufferIsFullException());
			}
			return data;
		}

		void DoPush(const ConstByteData& data)
		{
			WriteToStorage(_writeOffset, data.begin(), data.end());
			_writeOffset += data.size();
			if (_writeOffset == GetStorageSize())
				_writeOffset = 0;
		}

		void DoPop(size_t size)
		{
			_readOffset += size;
			if (_readOffset == GetStorageSize())
				_readOffset = 0;
		}

		void ReleaseData(size_t size)
		{
			MutexLock l(_mutex);

			if (_loggingEnabled)
			{
				s_logger.Warning() << "Release started";
				s_logger.Warning() << "ro: " << _readOffset << ", wo: " << _writeOffset << ", ls: " << _lockedDataSize;
			}

			if (_writeOffset >= _readOffset)
				DoPop(size);
			else
			{
				const size_t tailSize = GetStorageSize() - _readOffset;
				if (size <= tailSize)
					DoPop(size);
				else
				{
					DoPop(tailSize);
					DoPop(size - tailSize);
				}
			}

			_lockedDataSize = 0;

			if (_loggingEnabled)
			{
				s_logger.Warning() << "ro: " << _readOffset << ", wo: " << _writeOffset << ", ls: " << _lockedDataSize;
				s_logger.Warning() << "Release finished";
			}
		}

	public:
		CircularBufferBase(): _writeOffset(0), _readOffset(0), _lockedDataSize(0), _loggingEnabled(false)
		{ }

		void SetLoggingEnabled()
		{ _loggingEnabled = true; }

		shared_ptr<std::vector<u8> > GetAllData() const
		{
			shared_ptr<std::vector<u8> > result = make_shared_ptr<std::vector<u8> >();
			result->reserve(GetSize());

			if (_writeOffset >= _readOffset)
			{
				const ConstByteData src = ReadStorage(_readOffset, _writeOffset - _readOffset);
				std::copy(src.begin(), src.end(), std::back_inserter(*result));
			}
			else
			{
				{
					const ConstByteData src = ReadStorage(_readOffset, GetStorageSize() - _readOffset);
					std::copy(src.begin(), src.end(), std::back_inserter(*result));
				}
				{
					const ConstByteData src = ReadStorage(0, _writeOffset);
					std::copy(src.begin(), src.end(), std::back_inserter(*result));
				}
			}
			return result;
		}

		size_t GetSize() const
		{ return (_writeOffset >= _readOffset) ? (_writeOffset - _readOffset) : (GetStorageSize() - _readOffset + _writeOffset); }

		size_t GetFreeSize() const
		{ return (_writeOffset >= _readOffset) ? (GetStorageSize() - _writeOffset + _readOffset - 1) : (_readOffset - _writeOffset - 1); }

		CircularDataReserverPtr Pop(size_t size = std::numeric_limits<size_t>::max())
		{
			MutexLock l(_mutex);
			STINGRAYKIT_CHECK(_lockedDataSize == 0, "Previous data was not freed");

			if (_loggingEnabled)
			{
				s_logger.Warning() << "Pop started";
				s_logger.Warning() << "ro: " << _readOffset << ", wo: " << _writeOffset << ", ls: " << _lockedDataSize;
			}

			if (_writeOffset >= _readOffset)
				_lockedDataSize = std::min(size, _writeOffset - _readOffset);
			else
				_lockedDataSize = std::min(size, GetStorageSize() - _readOffset);

			if (_loggingEnabled)
			{
				s_logger.Warning() << "ro: " << _readOffset << ", wo: " << _writeOffset << ", ls: " << _lockedDataSize;
				s_logger.Warning() << "Pop finished";
			}
			return make_shared_ptr<CircularDataReserver>(ReadStorage(_readOffset, _lockedDataSize), Bind(&CircularBufferBase::ReleaseData, this, _1));
		}

		void Push(const ConstByteData& data)
		{
			MutexLock l(_mutex);
			if (_loggingEnabled)
			{
				s_logger.Warning() << "Push started";
				s_logger.Warning() << "ro: " << _readOffset << ", wo: " << _writeOffset << ", ls: " << _lockedDataSize;
			}
			const ConstByteData dataToPush(CheckDataSize(data));
			if (_writeOffset >= _readOffset)
			{
				const size_t tailCapacity = GetStorageSize() - _writeOffset;
				if (dataToPush.size() > tailCapacity)
				{
					DoPush(ConstByteData(dataToPush, 0, tailCapacity));
					DoPush(ConstByteData(dataToPush, tailCapacity, dataToPush.size() - tailCapacity));
				}
				else
					DoPush(dataToPush);
			}
			else
				DoPush(dataToPush);

			if (_loggingEnabled)
			{
				s_logger.Warning() << "ro: " << _readOffset << ", wo: " << _writeOffset << ", ls: " << _lockedDataSize;
				s_logger.Warning() << "Push finished";
			}
		}

		void Clear()
		{
			MutexLock l(_mutex);
			if (_loggingEnabled)
			{
				s_logger.Warning() << "Clear started";
				s_logger.Warning() << "ro: " << _readOffset << ", wo: " << _writeOffset << ", ls: " << _lockedDataSize;
			}

			if (_readOffset + _lockedDataSize < GetStorageSize())
				_writeOffset = _readOffset + _lockedDataSize;
			else
				_writeOffset = _readOffset + _lockedDataSize - GetStorageSize();

			if (_loggingEnabled)
			{
				s_logger.Warning() << "ro: " << _readOffset << ", wo: " << _writeOffset << ", ls: " << _lockedDataSize;
				s_logger.Warning() << "Clear finished";
			}
		}

		virtual bool CanPush(size_t size)
		{ return size <= GetFreeSize(); }
	};
	template<bool DiscardOnOverflow>
	STINGRAYKIT_DEFINE_NAMED_LOGGER(CircularBufferBase<DiscardOnOverflow>);

	/** @} */

}

#endif
