#include <stingray/stingraykit/stingraykit/io/PipeDataSource.h>

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
			consumer.EndOfData();
		}
	}

}
