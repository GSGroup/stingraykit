#ifndef STINGRAY_TOOLKIT_PAGEDBUFFER_H
#define STINGRAY_TOOLKIT_PAGEDBUFFER_H

#include <deque>

#include <stingray/toolkit/IByteStream.h>
#include <stingray/toolkit/ScopeExit.h>
#include <stingray/toolkit/ByteData.h>

namespace stingray
{


	class PagedBuffer
	{
	public:
		struct IPage
		{
			virtual ~IPage() { }

			virtual size_t Read(u64 offset, void* data, size_t count) = 0;
			virtual size_t Write(u64 offset, const void* data, size_t count) = 0;
		};
		typedef IPage Page;
		TOOLKIT_DECLARE_PTR(Page);

	private:
		typedef std::deque<PagePtr> PagesContainer;

	private:
		u64				_pageSize;
		PagesContainer	_pages;
		u64				_startOffset, _endOffset;
		Mutex			_mutex;
		bool			_pushing, _usingStart;

	public:
		PagedBuffer(u64 pageSize) :
			_pageSize(pageSize),
			_startOffset(0),
			_endOffset(0),
			_pushing(false),
			_usingStart(false)
		{ }

		virtual ~PagedBuffer()
		{ }

		void Push(const ConstByteData& data)
		{
			{
				MutexLock l(_mutex);
				TOOLKIT_CHECK(!_pushing, "Previous push has not finished yet!");
				_pushing = true;
			}
			ScopeExitInvoker sei(bind(&PagedBuffer::PushingFinished, this));

			u64 new_end_offset, page_idx = 0, page_write_size, page_offset;
			{
				page_write_size = std::min(_endOffset, (u64)data.size());
				page_offset = _endOffset == 0 ? 0 : _pageSize - _endOffset;

				for (; data.size() > _endOffset; _endOffset += _pageSize, ++page_idx)
					_pages.push_back(CreatePage(_pageSize));

				new_end_offset = _endOffset - data.size();
			}
			ScopeExitInvoker sei2(bind(&PagedBuffer::SetEndOffset, this, new_end_offset));

			WriteToPage(page_idx--, page_offset, ConstByteData(data, 0, page_write_size));

			for (u64 data_offset = page_write_size; data_offset < data.size(); data_offset += page_write_size, --page_idx)
			{
				page_write_size = std::min(_pageSize, (u64)data.size() - data_offset);
				WriteToPage(page_idx, 0, ConstByteData(data, data_offset, page_write_size));
			}
		}

		void Get(const ByteData& data)
		{
			u64 page_idx = 0, page_read_size, page_offset;
			{
				MutexLock l(_mutex);
				TOOLKIT_CHECK(data.size() <= GetSize(), IndexOutOfRangeException());

				TOOLKIT_CHECK(!_usingStart, "End is being used!");
				_usingStart = true;

				page_offset = _startOffset;
				page_read_size = std::min(_pageSize - _startOffset, (u64)data.size());
			}
			ScopeExitInvoker sei(bind(&PagedBuffer::ReleaseStart, this));

			u64 data_offset = 0;
			ReadFromPage(page_idx++, page_offset, ByteData(data, data_offset, page_read_size));
			data_offset += page_read_size;

			for (; data_offset < data.size(); data_offset += page_read_size, ++page_idx)
			{
				page_read_size = std::min(_pageSize, data.size() - data_offset);
				ReadFromPage(page_idx, 0, ByteData(data, data_offset, page_read_size));
			}
		}

		void Pop(u64 size)
		{
			MutexLock l(_mutex);

			TOOLKIT_CHECK(size <= GetSize(), IndexOutOfRangeException());
			TOOLKIT_CHECK(!_usingStart, "End is being used!");

			SetStartOffset(_startOffset + size);
		}

		u64 GetSize() const
		{
			MutexLock l(_mutex);
			return _pageSize * _pages.size() - _startOffset - _endOffset;
		}

	protected:
		virtual PagePtr CreatePage(u64 size) = 0;
		virtual void GCPage(PagePtr page) {}

	private:
		void PushingFinished()
		{
			MutexLock l(_mutex);
			_pushing = false;
		}

		void SetEndOffset(u64 newEndOffset)
		{
			MutexLock l(_mutex);
			_endOffset = newEndOffset;
		}

		void ReleaseStart()
		{
			MutexLock l(_mutex);
			_usingStart = false;
		}

		void SetStartOffset(u64 newStartOffset)
		{
			MutexLock l(_mutex);
			_startOffset = newStartOffset;

			for (; _startOffset >= _pageSize; _startOffset -= _pageSize)
			{
				GCPage(_pages.front());
				_pages.pop_front();
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
			if (p->Write(offsetInPage, &data[0], data.size()) != data.size())
				TOOLKIT_THROW("Page write failed!");
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
			if (p->Read(offsetInPage, &data[0], data.size()) != data.size())
				TOOLKIT_THROW("Page read failed!");
		}
	};



}


#endif

