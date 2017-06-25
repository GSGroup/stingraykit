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
		{ _source->ReadToFunction(bind(&DataAligner::Align, this, ref(consumer), _1, _2), bind(&IDataConsumer::EndOfData, ref(consumer), _1), token); }

	private:
		size_t Align(IDataConsumer& consumer, ConstByteData data, const ICancellationToken& token)
		{
			if (_buffer.empty())
			{
				const size_t processed = consumer.Process(ConstByteData(data, 0, AlignDown(data.size(), _alignment)), token);

				const size_t remainder = data.size() - processed;
				if (remainder && remainder < _alignment)
				{
					_buffer.append(ConstByteData(data, processed, remainder));
					return processed + remainder;
				}

				return processed;
			}

			const size_t offset = std::min(_alignment - _buffer.size(), data.size());

			_buffer.append(ConstByteData(data, 0, offset));

			if (_buffer.size() == _alignment)
			{
				STINGRAYKIT_CHECK(ConsumeAll(consumer, _buffer, token) == _alignment, "Could not process buffer");
				_buffer.clear();
			}

			return offset;
		}
	};

}

#endif
