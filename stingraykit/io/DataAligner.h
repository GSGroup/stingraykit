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

	public:
		DataAligner(const IDataSourcePtr& source, size_t alignment)
			:	_source(source),
				_alignment(alignment)
		{ }

		virtual void Read(IDataConsumer& consumer, const ICancellationToken& token)
		{ _source->ReadToFunction(Bind(&DataAligner::Align, this, wrap_ref(consumer), _1, _2), Bind(&IDataConsumer::EndOfData, wrap_ref(consumer), _1), token); }

	private:
		size_t Align(IDataConsumer& consumer, ConstByteData data, const ICancellationToken& token)
		{
			if (_buffer.empty())
			{
				size_t processed = ConsumeAll(consumer, ConstByteData(data, 0, AlignDown(data.size(), _alignment)), token);

				const size_t remainder = data.size() - processed;
				if (token && remainder)
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
				const size_t processed = ConsumeAll(consumer, _buffer, token);
				_buffer.clear();
				if (!token)
					return processed;
			}

			return offset;
		}
	};

}

#endif
