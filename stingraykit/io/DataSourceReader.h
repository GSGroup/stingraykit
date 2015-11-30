#ifndef STINGRAYKIT_IO_DATASOURCEREADER_H
#define STINGRAYKIT_IO_DATASOURCEREADER_H

// Copyright (c) 2011 - 2015, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <stingraykit/io/IDataSource.h>
#include <stingraykit/thread/DummyCancellationToken.h>


namespace stingray
{

	class DataSourceReader
	{
	private:
		IDataSourcePtr	_source;

	public:
		explicit DataSourceReader(const IDataSourcePtr& source);

		ByteArray ReadToEnd(const ICancellationToken& token = DummyCancellationToken());
	};

}


#endif
