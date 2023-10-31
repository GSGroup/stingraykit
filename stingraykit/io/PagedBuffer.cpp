// Copyright (c) 2011 - 2022, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/io/PagedBuffer.h>

#include <stingraykit/ScopeExit.h>

namespace stingray
{

	void PagedBuffer::Read(IDataConsumer& consumer, const ICancellationToken& token)
	{
		MutexLock lr(_readMutex);

		IPagePtr page;
		u64 offsetInPage;

		{
			MutexLock l(_mutex);
			page = _pages.at(_currentOffset / _pageSize);
			offsetInPage = _currentOffset % _pageSize;
		}

		const size_t processed = page->Read(offsetInPage, consumer, token);

		if (!token)
			return;

		MutexLock l(_mutex);
		_currentOffset += processed;
	}


	u64 PagedBuffer::GetSize(bool absolute) const
	{
		MutexLock lr(_readMutex);
		MutexLock l(_mutex);

		if (absolute)
			return _pageSize * _pages.size() - _startOffset - _tailSize;

		return _pageSize * _pages.size() - _currentOffset - _tailSize;
	}


	void PagedBuffer::Push(const ConstByteData& data, const ICancellationToken& token)
	{
		MutexLock lw(_writeMutex);

		u64 newTailSize;
		u64 pageIndexFromEnd = 0;
		u64 pageWriteSize;
		u64 offsetInPage;

		{
			MutexLock l(_mutex);

			pageWriteSize = std::min(_tailSize, (u64)data.size());
			offsetInPage = _tailSize == 0 ? 0 : _pageSize - _tailSize;

			for (; _tailSize < data.size(); _tailSize += _pageSize, ++pageIndexFromEnd)
				_pages.push_back(CreatePage());

			newTailSize = _tailSize - data.size();
		}

		PagedBuffer* self = this;
		STINGRAYKIT_SCOPE_EXIT(MK_PARAM(PagedBuffer*, self), MK_PARAM(u64, newTailSize))
			MutexLock l(self->_mutex);
			self->_tailSize = newTailSize;
		STINGRAYKIT_SCOPE_EXIT_END;

		WriteToPage(pageIndexFromEnd--, offsetInPage, ConstByteData(data, 0, pageWriteSize), token);

		for (u64 offset = pageWriteSize; offset < data.size(); offset += pageWriteSize, --pageIndexFromEnd)
		{
			pageWriteSize = std::min(_pageSize, (u64)data.size() - offset);
			WriteToPage(pageIndexFromEnd, 0, ConstByteData(data, offset, pageWriteSize), token);
		}
	}


	void PagedBuffer::Pop(u64 size)
	{
		MutexLock lr(_readMutex);
		MutexLock l(_mutex);

		const u64 absoluteSize = _pageSize * _pages.size() - _startOffset - _tailSize;
		STINGRAYKIT_CHECK(size <= absoluteSize, IndexOutOfRangeException(size, absoluteSize));

		_startOffset = _startOffset + size;
		if (_currentOffset < _startOffset)
			_currentOffset = _startOffset;

		for (; _startOffset >= _pageSize; _startOffset -= _pageSize, _currentOffset -= _pageSize)
			_pages.pop_front();
	}


	void PagedBuffer::Seek(u64 offset)
	{
		MutexLock lr(_readMutex);
		MutexLock l(_mutex);

		const u64 absoluteSize = _pageSize * _pages.size() - _startOffset - _tailSize;
		STINGRAYKIT_CHECK(offset <= absoluteSize, IndexOutOfRangeException(offset, absoluteSize));

		_currentOffset = _startOffset + offset;
	}


	PagedBuffer::PagedBuffer(u64 pageSize)
		:	_pageSize(pageSize),
			_startOffset(0),
			_currentOffset(0),
			_tailSize(0)
	{ }


	void PagedBuffer::WriteToPage(u64 pageIndexFromEnd, u64 offsetInPage, ConstByteData data, const ICancellationToken& token)
	{
		if (data.empty())
			return;

		IPagePtr page;
		{
			MutexLock l(_mutex);
			page = _pages.at(_pages.size() - pageIndexFromEnd - 1);
		}
		if (page->Write(offsetInPage, data, token) != data.size())
			STINGRAYKIT_THROW(InputOutputException("Page write failed!"));
	}

}
