// Copyright (c) 2011 - 2022, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/io/PagedBuffer.h>

namespace stingray
{

	class PagedBuffer::ReadLock
	{
	private:
		PagedBuffer&		_parent;

	public:
		ReadLock(PagedBuffer& parent) : _parent(parent)
		{
			STINGRAYKIT_CHECK(!_parent._activeRead, InvalidOperationException("Simultaneous Read()!"));
			_parent._activeRead = true;
		}

		~ReadLock()
		{ _parent._activeRead = false; }
	};


	class PagedBuffer::WriteGuard
	{
	private:
		PagedBuffer&		_parent;
		bool				_locked;

	public:
		WriteGuard(PagedBuffer& parent) : _parent(parent), _locked(false) { }

		ConditionWaitResult Wait(const ICancellationToken& token)
		{
			{
				MutexLock l(_parent._writeMutex);
				while (_parent._activeWrite)
				{
					const ConditionWaitResult result = _parent._writeCond.Wait(_parent._writeMutex, token);
					if (result != ConditionWaitResult::Broadcasted)
						return result;
				}

				_parent._activeWrite = true;
			}

			_locked = true;
			return ConditionWaitResult::Broadcasted;
		}

		~WriteGuard()
		{
			if (_locked)
			{
				MutexLock l(_parent._writeMutex);
				_parent._activeWrite = false;
				_parent._writeCond.Broadcast();
			}
		}
	};


	void PagedBuffer::Read(IDataConsumer& consumer, const ICancellationToken& token)
	{
		MutexLock l(_mutex);
		ReadLock rl(*this);

		const IPagePtr page = _pages.at(_currentOffset / _pageSize);
		const u64 offset = _currentOffset % _pageSize;

		const u64 currentOffset = _currentOffset;
		size_t processed = 0;

		{
			MutexUnlock ul(l);

			processed = page->Read(offset, consumer, token);
			if (!token)
				return;
		}

		if (_currentOffset == currentOffset)
			_currentOffset += processed;
	}


	u64 PagedBuffer::GetStorageSize() const
	{
		MutexLock l(_mutex);
		return _pageSize * _pages.size() - _startOffset - _tailSize;
	}


	u64 PagedBuffer::GetUnreadSize() const
	{
		MutexLock l(_mutex);
		return _pageSize * _pages.size() - _currentOffset - _tailSize;
	}


	void PagedBuffer::Push(const ConstByteData& data, const ICancellationToken& token)
	{
		WriteGuard g(*this);

		switch (g.Wait(token))
		{
		case ConditionWaitResult::Broadcasted:	break;
		case ConditionWaitResult::Cancelled:	STINGRAYKIT_THROW(OperationCancelledException());
		case ConditionWaitResult::TimedOut:		STINGRAYKIT_THROW(TimeoutException());
		}

		MutexLock l(_mutex);

		STINGRAYKIT_CHECK(!_pages.empty() || _tailSize == 0, LogicException("Broken invariant: no pages while tail size is non-zero"));

		size_t offset = 0;

		if (_tailSize > 0)
		{
			const IPagePtr tailPage = _pages.back();
			const size_t toWrite = std::min(_tailSize, (u64)data.size());

			{
				MutexUnlock ul(l);

				const size_t written = tailPage->Write(_pageSize - _tailSize, ConstByteData(data, offset, toWrite), token);
				STINGRAYKIT_CHECK(written == toWrite, InputOutputException(StringBuilder() % "Written only " % written % " of " % toWrite));
			}

			_tailSize -= toWrite;
			offset += toWrite;
		}

		std::vector<IPagePtr> newPages;
		u64 newTailSize = _tailSize;

		{
			MutexUnlock ul(l);

			while (offset < data.size())
			{
				newPages.push_back(CreatePage());

				const size_t toWrite = std::min(_pageSize, (u64)data.size() - offset);
				const size_t written = newPages.back()->Write(0, ConstByteData(data, offset, toWrite), token);
				STINGRAYKIT_CHECK(written == toWrite, InputOutputException(StringBuilder() % "Written only " % written % " of " % toWrite));

				newTailSize = _pageSize - toWrite;
				offset += toWrite;
			}
		}

		_pages.insert(_pages.end(), newPages.begin(), newPages.end());
		_tailSize = newTailSize;
	}


	void PagedBuffer::Pop(u64 size)
	{
		MutexLock l(_mutex);

		const u64 storageSize = GetStorageSize();
		STINGRAYKIT_CHECK(size <= storageSize, IndexOutOfRangeException(size, storageSize));

		auto newBeginIt = _pages.begin();
		u64 newStartOffset = _startOffset + size;
		u64 newCurrentOffset = std::max(newStartOffset, _currentOffset);

		while (newStartOffset >= _pageSize)
		{
			++newBeginIt;
			newStartOffset -= _pageSize;
			newCurrentOffset -= _pageSize;
		}

		_pages.erase(_pages.begin(), newBeginIt);
		_startOffset = newStartOffset;
		_currentOffset = newCurrentOffset;
	}


	void PagedBuffer::Seek(u64 offset)
	{
		MutexLock l(_mutex);

		const u64 storageSize = GetStorageSize();
		STINGRAYKIT_CHECK(offset <= storageSize, IndexOutOfRangeException(offset, storageSize));

		_currentOffset = _startOffset + offset;
	}


	PagedBuffer::PagedBuffer(u64 pageSize)
		:	_pageSize(pageSize),
			_startOffset(0),
			_currentOffset(0),
			_tailSize(0),
			_activeRead(false),
			_activeWrite(false)
	{ }

}
