#ifndef STINGRAYKIT_IO_BITHREADCIRCULARBUFFER_H
#define STINGRAYKIT_IO_BITHREADCIRCULARBUFFER_H

// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/ByteData.h>
#include <stingraykit/collection/BytesOwner.h>
#include <stingraykit/self_counter.h>

namespace stingray
{

	struct BithreadCircularBuffer
	{
		STINGRAYKIT_NONCOPYABLE(BithreadCircularBuffer);

	private:
		struct Impl;
		STINGRAYKIT_DECLARE_SELF_COUNT_PTR(Impl);

		ImplSelfCountPtr	_impl;

	public:
		struct Reader
		{
		private:
			ImplSelfCountPtr	_impl;
			ConstByteData		_data;

		public:
			Reader(const ImplSelfCountPtr& impl);
			Reader(const Reader&);
			~Reader();

			Reader& operator =(const Reader&);

			const u8* data() const;
			size_t size() const;
			ConstByteData::iterator begin();
			ConstByteData::iterator end();

			ConstByteData GetData() const;

			bool IsBufferEnd() const;

			void Pop(size_t bytes);
		};

		struct Writer
		{
		private:
			ImplSelfCountPtr	_impl;
			ByteData			_data;

		public:
			Writer(const ImplSelfCountPtr& impl);
			Writer(const Writer&);
			~Writer();

			Writer& operator =(const Writer&);

			u8* data() const;
			size_t size() const;
			ByteData::iterator begin();
			ByteData::iterator end();

			ByteData GetData() const;

			bool IsBufferEnd() const;

			void Push(size_t bytes);
		};

	public:
		BithreadCircularBuffer(size_t size);
		BithreadCircularBuffer(const BytesOwner& storage);
		~BithreadCircularBuffer();

		size_t GetDataSize() const;
		size_t GetFreeSize() const;

		size_t GetTotalSize() const;

		Reader Read();
		Writer Write();

		/// @brief: Clears buffer completely. Warning: can't be called simultaneously with Read(...) or Write(...)
		void Clear();
	};
	STINGRAYKIT_DECLARE_PTR(BithreadCircularBuffer);

}

#endif
