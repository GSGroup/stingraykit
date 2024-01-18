// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#ifndef STINGRAYKIT_IO_DATAALIGNER_H
#define STINGRAYKIT_IO_DATAALIGNER_H

#include <stingraykit/io/IDataSource.h>
#include <stingraykit/math.h>

namespace stingray
{

	class DataAligner : public virtual IDataSource
	{
	private:
		IDataSourcePtr	_source;
		size_t			_alignment;
		ByteArray		_buffer;
		bool			_useBuffer;

	public:
		DataAligner(const IDataSourcePtr& source, size_t alignment, bool useBuffer = true)
			:	_source(STINGRAYKIT_REQUIRE_NOT_NULL(source)),
				_alignment(alignment),
				_useBuffer(useBuffer)
		{ STINGRAYKIT_CHECK(_alignment != 0, ArgumentException("alignment")); }

		virtual void Read(IDataConsumer& consumer, const ICancellationToken& token)
		{ _source->ReadToFunction(Bind(&DataAligner::Align, this, wrap_ref(consumer), _1, _2), Bind(&IDataConsumer::EndOfData, wrap_ref(consumer), _1), token); }

	private:
		size_t Align(IDataConsumer& consumer, ConstByteData data, const ICancellationToken& token)
		{
			if (_buffer.empty())
			{
				size_t processed = ConsumeAll(consumer, ConstByteData(data, 0, AlignDown(data.size(), _alignment)), token);

				const size_t remainder = data.size() - processed;
				if (_useBuffer && token && remainder)
				{
					_buffer.append(ConstByteData(data, processed, remainder));
					processed += remainder;
				}

				return processed;
			}

			const size_t offset = std::min(_alignment - _buffer.size(), data.size());

			_buffer.append(ConstByteData(data, 0, offset));

			if (_buffer.size() == _alignment)
			{
				ConsumeAll(consumer, _buffer, token);
				_buffer.clear();
			}

			return offset;
		}
	};

}

#endif
