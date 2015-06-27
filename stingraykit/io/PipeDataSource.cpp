// Copyright (c) 2011 - 2015, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/io/PipeDataSource.h>

namespace stingray
{

	PipeDataSource::PipeDataSource(const IPipePtr& pipe, size_t bufferSize)
		: _pipe(pipe), _buffer(bufferSize)
	{ }


	void PipeDataSource::Read(IDataConsumer& consumer, const ICancellationToken& token)
	{
		try
		{
			const size_t read = _pipe->Read(_buffer.GetByteData(), token);
			if (read == 0)
				return;

			const size_t processed = consumer.Process(ConstByteData(_buffer, 0, read), token);
			if (processed == 0)
				return;

			STINGRAYKIT_CHECK(processed == read, NotImplementedException());
		}
		catch (const PipeClosedException&)
		{
			consumer.EndOfData(token);
		}
	}

}
