#ifndef STINGRAYKIT_IO_DATAUTILS_H
#define STINGRAYKIT_IO_DATAUTILS_H

#include <stingraykit/io/IDataSource.h>

#include <math.h>

namespace stingray
{

	class TakingConsumer : public virtual IDataConsumer
	{
	private:
		IDataConsumer&	_consumer;
		size_t			_leftToTake;
		bool			_discardRest;

	public:
		TakingConsumer(IDataConsumer& consumer, size_t count, bool discardRest = false)
			:	_consumer(consumer),
				_leftToTake(count),
				_discardRest(discardRest)
		{ }

		size_t Process(ConstByteData data, const ICancellationToken& token) override
		{
			const size_t size = std::min(_leftToTake, data.size());
			if (!size)
				return _discardRest ? data.size() : 0;

			const size_t processed = _consumer.Process(ConstByteData(data, 0, size), token);
			_leftToTake -= processed;

			return processed;
		}

		void EndOfData(const ICancellationToken& token) override
		{
			_leftToTake = 0;
			_consumer.EndOfData(token);
		}

		size_t GetLeftToTake() const
		{ return _leftToTake; }
	};


	class DataTaker : public virtual IDataSource
	{
	private:
		IDataSourcePtr	_source;
		size_t			_leftToTake;
		bool			_discardRest;

	public:
		DataTaker(const IDataSourcePtr& source, size_t count, bool discardRest = false)
			:	_source(source),
				_leftToTake(count),
				_discardRest(discardRest)
		{ }

		void Read(IDataConsumer& consumer, const ICancellationToken& token) override
		{
			TakingConsumer taker(consumer, _leftToTake, _discardRest);
			while (token && taker.GetLeftToTake())
				_source->Read(taker, token);
			_leftToTake = taker.GetLeftToTake();
		}
	};

}

#endif
