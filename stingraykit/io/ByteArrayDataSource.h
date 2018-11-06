#ifndef STINGRAYKIT_IO_BYTEARRAYDATASOURCE_H
#define STINGRAYKIT_IO_BYTEARRAYDATASOURCE_H

// Copyright (c) 2011 - 2018, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/io/IDataSource.h>

namespace stingray
{

	class ByteArrayDataSource : public virtual IDataSource
	{
	private:
		ConstByteArray	_data;
		size_t			_processed;

	public:
		explicit ByteArrayDataSource(const ConstByteArray& data)
			: _data(data), _processed(0)
		{ }

		virtual void Read(IDataConsumer& consumer, const ICancellationToken& token)
		{
			if (_processed < _data.size())
				_processed += consumer.Process(ConstByteData(_data, _processed), token);
			else
				consumer.EndOfData(token);
		}
	};

}

#endif
