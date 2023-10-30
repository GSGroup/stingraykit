#ifndef STINGRAYKIT_IO_PAGEDBUFFER_H
#define STINGRAYKIT_IO_PAGEDBUFFER_H

// Copyright (c) 2011 - 2022, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/io/IDataSource.h>
#include <stingraykit/ScopeExit.h>

#include <deque>

namespace stingray
{

	class PagedBuffer : public virtual IDataSource
	{
	protected:
		struct IPage
		{
			virtual ~IPage() { }

			virtual size_t Read(u64 offset, IDataConsumer& consumer, const ICancellationToken& token) = 0;
			virtual size_t Write(u64 offset, ConstByteData data) = 0;
		};
		STINGRAYKIT_DECLARE_PTR(IPage);

	private:
		using PagesContainer = std::deque<IPagePtr>;

	private:
		u64							_pageSize;
		PagesContainer				_pages;

		u64							_startOffset;
		u64							_endOffset;
		u64							_popOffset;

		Mutex						_readMutex;
		Mutex						_writeMutex;
		Mutex						_mutex;

	public:
		void Read(IDataConsumer& consumer, const ICancellationToken& token) override
		{
			MutexLock lr(_readMutex);

			u64 pageIndex;
			u64 offsetInPage;

			{
				MutexLock l(_mutex);

				pageIndex = _startOffset / _pageSize;
				offsetInPage = _startOffset % _pageSize;
			}

			const size_t processed = ReadFromPage(pageIndex, offsetInPage, consumer, token);

			if (!token)
				return;

			MutexLock l(_mutex);
			_startOffset += processed;
		}

		u64 GetSize(bool absolute = false) const
		{
			MutexLock lr(_readMutex);
			MutexLock l(_mutex);

			if (absolute)
				return _pageSize * _pages.size() - _endOffset - _popOffset;

			return _pageSize * _pages.size() - _startOffset - _endOffset;
		}

		void Push(const ConstByteData& data)
		{
			MutexLock lw(_writeMutex);

			u64 newEndOffset;
			u64 pageIndexFromEnd = 0;
			u64 pageWriteSize;
			u64 offsetInPage;

			{
				MutexLock l(_mutex);

				pageWriteSize = std::min(_endOffset, (u64)data.size());
				offsetInPage = _endOffset == 0 ? 0 : _pageSize - _endOffset;

				for (; data.size() > _endOffset; _endOffset += _pageSize, ++pageIndexFromEnd)
					_pages.push_back(CreatePage());

				newEndOffset = _endOffset - data.size();
			}

			ScopeExitInvoker sei(Bind(&PagedBuffer::SetEndOffset, this, newEndOffset));

			WriteToPage(pageIndexFromEnd--, offsetInPage, ConstByteData(data, 0, pageWriteSize));

			for (u64 offset = pageWriteSize; offset < data.size(); offset += pageWriteSize, --pageIndexFromEnd)
			{
				pageWriteSize = std::min(_pageSize, (u64)data.size() - offset);
				WriteToPage(pageIndexFromEnd, 0, ConstByteData(data, offset, pageWriteSize));
			}
		}

		void Pop(u64 size)
		{
			MutexLock lr(_readMutex);
			MutexLock l(_mutex);

			const u64 absoluteSize = _pageSize * _pages.size() - _endOffset - _popOffset;
			STINGRAYKIT_CHECK(size <= absoluteSize, IndexOutOfRangeException(size, absoluteSize));

			SetPopOffset(_popOffset + size);
		}

		void Seek(u64 offset)
		{
			MutexLock lr(_readMutex);
			MutexLock l(_mutex);

			STINGRAYKIT_CHECK(offset <= _pageSize * _pages.size() - _endOffset - _popOffset, IndexOutOfRangeException(offset, _pageSize * _pages.size() - _endOffset - _popOffset));
			_startOffset = _popOffset + offset;
		}

	protected:
		explicit PagedBuffer(u64 pageSize)
			:	_pageSize(pageSize),
				_startOffset(0),
				_endOffset(0),
				_popOffset(0)
		{ }

		virtual IPagePtr CreatePage() = 0;

	private:
		void SetEndOffset(u64 newEndOffset)
		{
			MutexLock l(_mutex);
			_endOffset = newEndOffset;
		}

		void SetPopOffset(u64 newPopOffset)
		{
			MutexLock l(_mutex);

			_popOffset = newPopOffset;
			if (_startOffset < _popOffset)
				_startOffset = _popOffset;

			for (; _popOffset >= _pageSize; _popOffset -= _pageSize, _startOffset -= _pageSize)
			{
				STINGRAYKIT_CHECK(_startOffset >= _pageSize, StringBuilder() % "Internal error: _startOffset is less than _pageSize while popping. _startOffset: " % _startOffset % ", _pageSize: " % _pageSize % ".");
				_pages.pop_front();
			}
		}

		void WriteToPage(u64 pageIndexFromEnd, u64 offsetInPage, ConstByteData data)
		{
			if (data.empty())
				return;

			IPagePtr page;
			{
				MutexLock l(_mutex);
				page = _pages.at(_pages.size() - pageIndexFromEnd - 1);
			}
			if (page->Write(offsetInPage, data) != data.size())
				STINGRAYKIT_THROW(InputOutputException("Page write failed!"));
		}

		size_t ReadFromPage(u64 pageIndex, u64 offsetInPage, IDataConsumer& consumer, const ICancellationToken& token) const
		{
			IPagePtr page;
			{
				MutexLock l(_mutex);
				page = _pages.at(pageIndex);
			}
			return page->Read(offsetInPage, consumer, token);
		}
	};

}

#endif
