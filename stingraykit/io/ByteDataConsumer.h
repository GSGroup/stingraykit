// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#ifndef STINGRAYKIT_IO_BYTEDATACONSUMER_H
#define STINGRAYKIT_IO_BYTEDATACONSUMER_H

#include <stingraykit/io/IDataSource.h>

namespace stingray
{

	class ByteDataConsumer : public virtual IDataConsumer
	{
	private:
		ByteData			_destination;
		bool				_eod;

	public:
		explicit ByteDataConsumer(ByteData destination);

		size_t Process(ConstByteData data, const ICancellationToken&) override;
		void EndOfData(const ICancellationToken&) override { _eod = true; }

		bool IsFull() const { return _destination.empty(); }
		bool IsEndOfData() const { return _eod; }
	};
	STINGRAYKIT_DECLARE_PTR(ByteDataConsumer);

}

#endif
