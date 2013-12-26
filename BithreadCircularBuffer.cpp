#include <stingray/toolkit/BithreadCircularBuffer.h>

#include <vector>

#include <stingray/threads/Thread.h>

namespace stingray
{

	struct BithreadCircularBuffer::Impl
	{
		TOOLKIT_NONCOPYABLE(Impl);

		// TODO: finish padding support
		static const size_t PaddingSize = 8;

	private:
		typedef std::vector<u8> ContainerType;

		Mutex			_mutex;
		ContainerType	_container;
		size_t			_writeOffset, _readOffset;
		size_t			_lockedForWrite, _lockedForRead;
		atomic_int_type	_readersCount, _writersCount;
		bool			_dataIsContiguous;

	public:
		Impl(size_t size) :
			_container(size), _writeOffset(0), _readOffset(0),
			_lockedForWrite(0), _lockedForRead(0),
			_readersCount(0), _writersCount(0),
			_dataIsContiguous(true)
		{}


		size_t GetDataSize() const
		{
			MutexLock l(_mutex);
			return _dataIsContiguous ? (_writeOffset - _readOffset) : (GetStorageSize() - _readOffset + _writeOffset);
		}


		size_t GetFreeSize() const
		{
			MutexLock l(_mutex);
			return GetStorageSize() - GetDataSize() - 1;
		}


		size_t GetStorageSize() const
		{ return _container.size(); }


		ByteData LockForWrite()
		{
			MutexLock l(_mutex);
			TOOLKIT_CHECK(_lockedForWrite == 0, "There is another write in progress!");

			_lockedForWrite = _dataIsContiguous ? (GetStorageSize() - _writeOffset) : (_readOffset - _writeOffset);
			return ByteData(&_container[_writeOffset], _lockedForWrite);
		}


		void UnlockWriteAndPush(size_t pushSize)
		{
			MutexLock l(_mutex);
			TOOLKIT_CHECK(pushSize <= _lockedForWrite, ArgumentException("pushSize", pushSize));
			_writeOffset += pushSize;

			if (_writeOffset == GetStorageSize())
			{
				_writeOffset = 0;
				_dataIsContiguous = !_dataIsContiguous;
			}

			_lockedForWrite = 0;
		}


		ConstByteData LockForRead()
		{
			MutexLock l(_mutex);
			TOOLKIT_CHECK(_lockedForRead == 0, "There is another read in progress!");

			_lockedForRead = (_dataIsContiguous) ? (_writeOffset - _readOffset) : (GetStorageSize() - _readOffset);
			return ByteData(&_container[_readOffset], _lockedForRead);
		}


		void UnlockReadAndPop(size_t popSize)
		{
			MutexLock l(_mutex);
			TOOLKIT_CHECK(popSize <= _lockedForRead, ArgumentException("popSize", popSize));
			_readOffset += popSize;

			if (_readOffset == GetStorageSize())
			{
				_readOffset = 0;
				_dataIsContiguous = !_dataIsContiguous;
			}

			_lockedForRead = 0;
		}


		atomic_int_type& ReadersCount() { return _readersCount; }
		atomic_int_type& WritersCount() { return _writersCount; }
	};


	BithreadCircularBuffer::BithreadCircularBuffer(size_t size) : _impl(new Impl(size))
	{ }


	BithreadCircularBuffer::~BithreadCircularBuffer()
	{}


	size_t BithreadCircularBuffer::GetDataSize()
	{ return _impl->GetDataSize(); }


	size_t BithreadCircularBuffer::GetFreeSize()
	{ return _impl->GetFreeSize(); }


	size_t BithreadCircularBuffer::GetTotalSize() const
	{ return _impl->GetStorageSize(); }


	BithreadCircularBuffer::Reader BithreadCircularBuffer::Read()
	{ return Reader(_impl); }


	BithreadCircularBuffer::Writer BithreadCircularBuffer::Write()
	{ return Writer(_impl); }


	BithreadCircularBuffer::Reader::Reader(const ImplPtr& impl) :
		_impl(impl),
		_data(impl->LockForRead()),
		_valid(true)
	{ Atomic::Inc(_impl->ReadersCount()); }


	BithreadCircularBuffer::Reader::Reader(const Reader& other) : _impl(other._impl), _data(other._data)
	{ Atomic::Inc(_impl->ReadersCount()); }


	BithreadCircularBuffer::Reader::~Reader()
	{
		if (Atomic::Dec(_impl->ReadersCount()) == 0)
			_impl->UnlockReadAndPop(0);
	}


	size_t BithreadCircularBuffer::Reader::size()
	{ return _data.size(); }


	ConstByteData::iterator BithreadCircularBuffer::Reader::begin()
	{ TOOLKIT_CHECK(_valid, "Already popped data!"); return _data.begin(); }


	ConstByteData::iterator BithreadCircularBuffer::Reader::end()
	{ TOOLKIT_CHECK(_valid, "Already popped data!"); return _data.end(); }


	ConstByteData BithreadCircularBuffer::Reader::GetData()
	{ TOOLKIT_CHECK(_valid, "Already popped data!"); return _data; }


	void BithreadCircularBuffer::Reader::Pop(size_t bytes)
	{
		_impl->UnlockReadAndPop(bytes);
		_valid = false;
	}


	BithreadCircularBuffer::Writer::Writer(const ImplPtr& impl) :
		_impl(impl),
		_data(impl->LockForWrite()),
		_valid(true)
	{ Atomic::Inc(_impl->WritersCount()); }


	BithreadCircularBuffer::Writer::Writer(const Writer& other) : _impl(other._impl), _data(other._data)
	{ Atomic::Inc(_impl->WritersCount()); }


	BithreadCircularBuffer::Writer::~Writer()
	{
		if (Atomic::Dec(_impl->WritersCount()) == 0)
			_impl->UnlockWriteAndPush(0);
	}

	size_t BithreadCircularBuffer::Writer::size()
	{ return _data.size(); }


	ByteData::iterator BithreadCircularBuffer::Writer::begin()
	{ TOOLKIT_CHECK(_valid, "Already pushed data!"); return _data.begin(); }


	ByteData::iterator BithreadCircularBuffer::Writer::end()
	{ TOOLKIT_CHECK(_valid, "Already pushed data!"); return _data.end(); }


	ByteData BithreadCircularBuffer::Writer::GetData()
	{ TOOLKIT_CHECK(_valid, "Already pushed data!"); return _data; }


	void BithreadCircularBuffer::Writer::Push(size_t bytes)
	{
		_impl->UnlockWriteAndPush(bytes);
		_valid = false;
	}

}
