#ifndef __GS_STINGRAY_TOOLKIT_PAGEDBUFFER_H__
#define __GS_STINGRAY_TOOLKIT_PAGEDBUFFER_H__

#include <deque>

#include <stingray/toolkit/IByteStream.h>
#include <stingray/toolkit/ScopeExit.h>

namespace stingray
{


	class PagedBuffer
	{
	public:
		struct IPage
		{
			virtual ~IPage() { }

			virtual size_t Read(size_t offset, void* data, size_t count) = 0;
			virtual size_t Write(size_t offset, const void* data, size_t count) = 0;
		};
		typedef IPage Page;
		TOOLKIT_DECLARE_PTR(Page);

	private:
		typedef std::deque<PagePtr> PagesContainer;

	private:
		size_t			_pageSize;
		PagesContainer	_pages;
		size_t			_startOffset, _endOffset;
		Mutex			_mutex;
		bool			_pushing, _popping;

	public:
		PagedBuffer(size_t pageSize) :
			_pageSize(pageSize),
			_startOffset(0),
			_endOffset(0),
			_pushing(false),
			_popping(false)
		{
		}

		virtual ~PagedBuffer()
		{
		}

		void Push(const ConstByteData& data)
		{
			size_t new_end_offset, page_idx = 0, page_write_size, page_offset;
			{
				MutexLock l(_mutex);
				TOOLKIT_CHECK(!_pushing, "Previous push has not finished yet!");
				_pushing = true;

				page_write_size = std::min(_endOffset, data.size());
				page_offset = _endOffset == 0 ? 0 : _pageSize - _endOffset;

				for (; data.size() > _endOffset; _endOffset += _pageSize, ++page_idx)
					_pages.push_back(CreatePage(_pageSize));

				new_end_offset = _endOffset - data.size();
			}
			ScopeExitInvoker sei(bind(&PagedBuffer::PushingFinished, this, new_end_offset));

			size_t data_offset = 0;

			WriteToPage(page_idx--, page_offset, ConstByteData(data, data_offset, page_write_size));
			data_offset += page_write_size;

			for (; data_offset < data.size(); data_offset += page_write_size, --page_idx)
			{
				page_write_size = std::min(_pageSize, data.size() - data_offset);
				WriteToPage(page_idx, 0, ConstByteData(data, data_offset, page_write_size));
			}
		}

		void Pop(const ByteData& data)
		{
			size_t new_start_offset, page_idx = 0, page_read_size, page_offset;
			{
				MutexLock l(_mutex);
				TOOLKIT_CHECK(!_popping, "Previous pop has not finished yet!");
				_popping = true;

				TOOLKIT_CHECK(data.size() <= GetSize(), IndexOutOfRangeException());
				new_start_offset = _startOffset + data.size();
				page_offset = _startOffset;
				page_read_size = std::min(_pageSize - _startOffset, data.size());
			}
			ScopeExitInvoker sei(bind(&PagedBuffer::PoppingFinished, this, new_start_offset));

			size_t data_offset = 0;
			ReadFromPage(page_idx++, page_offset, ByteData(data, data_offset, page_read_size));
			data_offset += page_read_size;

			for (; data_offset < data.size(); data_offset += page_read_size, ++page_idx)
			{
				page_read_size = std::min(_pageSize, data.size() - data_offset);
				ReadFromPage(page_idx, 0, ByteData(data, data_offset, page_read_size));
			}
		}

		size_t GetSize() const
		{
			MutexLock l(_mutex);
			return _pageSize * _pages.size() - _startOffset - _endOffset;
		}

	protected:
		virtual PagePtr CreatePage(size_t size) = 0;
		virtual void GCPage(PagePtr page) {}

	private:
		void PushingFinished(size_t newEndOffset)
		{
			MutexLock l(_mutex);
			_endOffset = newEndOffset;
			_pushing = false;
		}

		void PoppingFinished(size_t newStartOffset)
		{
			MutexLock l(_mutex);
			_startOffset = newStartOffset;

			for (; _startOffset >= _pageSize; _startOffset -= _pageSize)
			{
				GCPage(_pages.front());
				_pages.pop_front();
			}

			_popping = false;
		}

		void WriteToPage(size_t pageIdxFromEnd, size_t offsetInPage, ConstByteData data)
		{
			if (data.empty())
				return;

			PagePtr p;
			{
				MutexLock l(_mutex);
				p = _pages.at(_pages.size() - pageIdxFromEnd - 1);
			}
			p->Write(offsetInPage, &data[0], data.size());
		}

		void ReadFromPage(size_t pageIdxFromStart, size_t offsetInPage, ByteData data) const
		{
			if (data.empty())
				return;

			PagePtr p;
			{
				MutexLock l(_mutex);
				p = _pages.at(pageIdxFromStart);
			}
			p->Read(offsetInPage, &data[0], data.size());
		}
	};



}


#endif

