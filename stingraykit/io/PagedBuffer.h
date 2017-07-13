#ifndef STINGRAYKIT_IO_PAGEDBUFFER_H
#define STINGRAYKIT_IO_PAGEDBUFFER_H

// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <deque>

#include <stingraykit/ScopeExit.h>
#include <stingraykit/collection/ByteData.h>
#include <stingraykit/io/IDataSource.h>
#include <stingraykit/io/ByteDataConsumer.h>
#include <stingraykit/signal/signals.h>
#include <stingraykit/thread/DummyCancellationToken.h>

namespace stingray
{


	class PagedBuffer : public virtual IDataSource
	{
	public:
		struct IPage
		{
			virtual ~IPage() { }

			virtual size_t Read(u64 offset, IDataConsumer& consumer, const ICancellationToken& token) = 0;
			virtual size_t Write(u64 offset, ConstByteData data) = 0;
		};
		typedef IPage Page;
		STINGRAYKIT_DECLARE_PTR(Page);

	private:
		typedef std::deque<PagePtr> PagesContainer;

	private:
		u64				_pageSize;
		PagesContainer	_pages;
		u64				_startOffset, _endOffset, _popOffset;
		Mutex			_readMutex;
		Mutex			_writeMutex;
		Mutex			_mutex;

	public:
		PagedBuffer(u64 pageSize) :
			_pageSize(pageSize),
			_startOffset(0),
			_endOffset(0),
			_popOffset(0)
		{ }

		virtual ~PagedBuffer()
		{ }

		void Push(const ConstByteData& data)
		{
			MutexLock lw(_writeMutex);

			u64 new_end_offset, page_idx = 0, page_write_size, page_offset;
			{
				MutexLock l(_mutex);

				page_write_size = std::min(_endOffset, (u64)data.size());
				page_offset = _endOffset == 0 ? 0 : _pageSize - _endOffset;

				for (; data.size() > _endOffset; _endOffset += _pageSize, ++page_idx)
					_pages.push_back(CreatePage());

				new_end_offset = _endOffset - data.size();
			}
			ScopeExitInvoker sei(bind(&PagedBuffer::SetEndOffset, this, new_end_offset));

			WriteToPage(page_idx--, page_offset, ConstByteData(data, 0, page_write_size));

			for (u64 data_offset = page_write_size; data_offset < data.size(); data_offset += page_write_size, --page_idx)
			{
				page_write_size = std::min(_pageSize, (u64)data.size() - data_offset);
				WriteToPage(page_idx, 0, ConstByteData(data, data_offset, page_write_size));
			}
		}

		void Get(const ByteData& data)
		{
			MutexLock lr(_readMutex);

			u64 page_idx, page_read_size, page_offset;
			{
				MutexLock l(_mutex);

				STINGRAYKIT_CHECK(data.size() <= GetSize(), IndexOutOfRangeException());

				page_idx = _startOffset / _pageSize;
				page_offset = _startOffset % _pageSize;
				page_read_size = std::min(_pageSize - _startOffset % _pageSize, (u64)data.size());
			}

			u64 data_offset = 0;
			ReadFromPage(page_idx++, page_offset, ByteData(data, data_offset, page_read_size));
			data_offset += page_read_size;

			for (; data_offset < data.size(); data_offset += page_read_size, ++page_idx)
			{
				page_read_size = std::min(_pageSize, data.size() - data_offset);
				ReadFromPage(page_idx, 0, ByteData(data, data_offset, page_read_size));
			}

			{
				MutexLock l(_mutex);
				_startOffset += data.size();
			}
		}

		virtual void Read(IDataConsumer& consumer, const ICancellationToken& token)
		{
			MutexLock lr(_readMutex);

			u64 page_idx, page_offset;
			{
				MutexLock l(_mutex);

				page_idx = _startOffset / _pageSize;
				page_offset = _startOffset % _pageSize;
			}

			size_t consumed = ReadFromPage(page_idx, page_offset, consumer, token);

			if (!token)
				return;

			{
				MutexLock l(_mutex);
				_startOffset += consumed;
			}
		}

		void Seek(u64 offset)
		{
			MutexLock lr(_readMutex);
			MutexLock l(_mutex);
			STINGRAYKIT_CHECK(offset <= _pageSize * _pages.size() - _endOffset - _popOffset, IndexOutOfRangeException(offset, _pageSize * _pages.size() - _endOffset - _popOffset));

			_startOffset = _popOffset + offset;
		}

		void Pop(u64 size)
		{
			MutexLock lr(_readMutex);
			MutexLock l(_mutex);

			const u64 absoluteSize = _pageSize * _pages.size() - _endOffset - _popOffset;
			STINGRAYKIT_CHECK(size <= absoluteSize, IndexOutOfRangeException(size, absoluteSize));

			SetPopOffset(_popOffset + size);
		}

		u64 GetSize(bool absolute = false) const
		{
			MutexLock lr(_readMutex);
			MutexLock l(_mutex);

			if (absolute)
				return _pageSize * _pages.size() - _endOffset - _popOffset;

			return _pageSize * _pages.size() - _startOffset - _endOffset;
		}

		signal<void()> OnDiscontinuity;

	private:
		virtual PagePtr CreatePage() = 0;
		virtual void GCPage(PagePtr page) {}

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

			for (; _popOffset >= _pageSize; _popOffset -= _pageSize)
			{
				GCPage(_pages.front());
				_pages.pop_front();

				if (_pageSize > _startOffset)
				{
					_startOffset = 0;
					OnDiscontinuity();
				}
				else
					_startOffset -= _pageSize;
			}
		}

		void WriteToPage(u64 pageIdxFromEnd, u64 offsetInPage, ConstByteData data)
		{
			if (data.empty())
				return;

			PagePtr p;
			{
				MutexLock l(_mutex);
				p = _pages.at(_pages.size() - pageIdxFromEnd - 1);
			}
			if (p->Write(offsetInPage, data) != data.size())
				STINGRAYKIT_THROW(InputOutputException("Page write failed!"));
		}

		void ReadFromPage(u64 pageIdxFromStart, u64 offsetInPage, ByteData data) const
		{
			if (data.empty())
				return;

			PagePtr p;
			{
				MutexLock l(_mutex);
				p = _pages.at(pageIdxFromStart);
			}
			ByteDataConsumer consumer(data);
			if (p->Read(offsetInPage, consumer, DummyCancellationToken()) != data.size())
				STINGRAYKIT_THROW(InputOutputException("Page read failed!"));
		}

		size_t ReadFromPage(u64 pageIdxFromStart, u64 offsetInPage, IDataConsumer& consumer, const ICancellationToken& token) const
		{
			PagePtr p;
			{
				MutexLock l(_mutex);
				p = _pages.at(pageIdxFromStart);
			}
			return p->Read(offsetInPage, consumer, token);
		}
	};


}


#endif

