#ifndef STINGRAYKIT_IO_PAGEDBUFFER_H
#define STINGRAYKIT_IO_PAGEDBUFFER_H

// Copyright (c) 2011 - 2022, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/io/IDataSource.h>

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
		void Read(IDataConsumer& consumer, const ICancellationToken& token) override;

		u64 GetSize(bool absolute = false) const;

		void Push(const ConstByteData& data);
		void Pop(u64 size);

		void Seek(u64 offset);

	protected:
		explicit PagedBuffer(u64 pageSize);

		virtual IPagePtr CreatePage() = 0;

	private:
		void WriteToPage(u64 pageIndexFromEnd, u64 offsetInPage, ConstByteData data);
	};

}

#endif
