#ifndef STINGRAY_TOOLKIT_IO_CIRCULARBUFFERBASE_H
#define STINGRAY_TOOLKIT_IO_CIRCULARBUFFERBASE_H


#include <limits>
#include <algorithm>

#include <stingray/toolkit/log/Logger.h>
#include <stingray/toolkit/io/ICircularBuffer.h>
#include <stingray/toolkit/collection/ByteData.h>
#include <stingray/toolkit/function/bind.h>


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
		size_t	_writeOffset, _readOffset, _lockedDataSize;
		Mutex	_mutex;
		static NamedLogger s_logger;
		bool _loggingEnabled;

		ConstByteData CheckDataSize(const ConstByteData &data)
		{ // Crops data if necessary
			size_t total_capacity = (_writeOffset >= _readOffset) ? (GetStorageSize() - _writeOffset + _readOffset - 1)
																  : (_readOffset - _writeOffset - 1);
			if (data.size() > total_capacity)
			{
				if (DiscardOnOverflow)
				{
					size_t result_size = std::min(data.size(), GetStorageSize() - 1);
					ConstByteData cropped_data(data, data.size() - result_size, result_size);
					Pop(result_size - total_capacity);
					return cropped_data;
				}
				else
					TOOLKIT_THROW(BufferIsFullException());
			}
			return data;
		}

		void DoPush(const ConstByteData &data)
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

			if (_writeOffset >= _readOffset) // data is stored in the middle of the container?
				DoPop(size);
			else
			{
				size_t tail_size = GetStorageSize() - _readOffset;
				if (size <= tail_size) // pop part of tail?
					DoPop(size);
				else
				{
					DoPop(tail_size);
					DoPop(size - tail_size);
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
		{
			_loggingEnabled = true;
		}

		shared_ptr<std::vector<u8> > GetAllData() const
		{
			shared_ptr<std::vector<u8> > result(new std::vector<u8>);
			result->reserve(GetSize());

			if (_writeOffset >= _readOffset)
			{
				ConstByteData src = ReadStorage(_readOffset, _writeOffset - _readOffset);
				std::copy(src.begin(), src.end(), std::back_inserter(*result));
			}
			else
			{
				{
					ConstByteData src = ReadStorage(_readOffset, GetStorageSize() - _readOffset);
					std::copy(src.begin(), src.end(), std::back_inserter(*result));
				}
				{
					ConstByteData src = ReadStorage(0, _writeOffset);
					std::copy(src.begin(), src.end(), std::back_inserter(*result));
				}
			}
			return result;
		}

		size_t GetSize() const
		{
			size_t total_data_size = (_writeOffset >= _readOffset) ? (_writeOffset - _readOffset)
																   : (GetStorageSize() - _readOffset + _writeOffset);
			return total_data_size;
		}

		size_t GetFreeSize() const
		{ return  (_writeOffset >= _readOffset) ? (GetStorageSize() - _writeOffset + _readOffset - 1) : (_readOffset - _writeOffset - 1); }

		CircularDataReserverPtr Pop(size_t size = std::numeric_limits<size_t>::max())
		{
			MutexLock l(_mutex);
			TOOLKIT_CHECK(_lockedDataSize == 0, "Previous data was not freed");

			if (_loggingEnabled)
			{
				s_logger.Warning() << "Pop started";
				s_logger.Warning() << "ro: " << _readOffset << ", wo: " << _writeOffset << ", ls: " << _lockedDataSize;
			}

			size_t result_size = size;
			if (_writeOffset >= _readOffset)
				result_size = std::min(result_size, _writeOffset - _readOffset);
			else
				result_size = std::min(result_size, GetStorageSize() - _readOffset);

			_lockedDataSize = result_size;

			if (_loggingEnabled)
			{
				s_logger.Warning() << "ro: " << _readOffset << ", wo: " << _writeOffset << ", ls: " << _lockedDataSize;
				s_logger.Warning() << "Pop finished";
			}
			return make_shared<CircularDataReserver>(ReadStorage(_readOffset, result_size), bind(&CircularBufferBase::ReleaseData, this, _1));
		}

		void Push(const ConstByteData &data)
		{
			MutexLock l(_mutex);
			if (_loggingEnabled)
			{
				s_logger.Warning() << "Push started";
				s_logger.Warning() << "ro: " << _readOffset << ", wo: " << _writeOffset << ", ls: " << _lockedDataSize;
			}
			ConstByteData data_to_push(CheckDataSize(data));
			if (_writeOffset >= _readOffset) // data is stored in the middle of the container?
			{
				size_t tail_capacity = GetStorageSize() - _writeOffset; // tail capacity
				if (data_to_push.size() > tail_capacity)
				{ // split pushed data in two pieces
					DoPush(ConstByteData(data_to_push, 0, tail_capacity));
					DoPush(ConstByteData(data_to_push, tail_capacity, data_to_push.size() - tail_capacity));
				}
				else
					DoPush(data_to_push);
			}
			else
				DoPush(data_to_push);

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
	TOOLKIT_DEFINE_NAMED_LOGGER(CircularBufferBase<DiscardOnOverflow>);

	/** @} */

}


#endif
