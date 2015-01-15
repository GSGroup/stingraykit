#ifndef STINGRAYKIT_ACCUMULATORS_H
#define STINGRAYKIT_ACCUMULATORS_H

#include <stingraykit/DataPipe.h>

namespace stingray {

	/**
	 * @addtogroup core_parsers
	 * @{
	 */

	// WARNING - Accumulators WORK FOR SIGNED TYPES ONLY!
	template<typename DataType>
	struct Accumulators
	{
		struct Mean : DataPipeStageBase<DataType, DataType>
		{
		private:
			DataType	_mean;
			DataType	_count;

		public:
			Mean() : _mean(0), _count(0)
			{}

			template<typename Callback>
			void Process(DataType val, const Callback& cb)
			{
				_mean = _mean + (val - _mean) / ++_count;
				cb(val);
			}
			DataType Get() const	{ return _mean; }
			void Reset()			{ _count = _mean = 0; }
		};

		struct Deviation : DataPipeStageBase<DataType, DataType>
		{
		private:
			DataType	_mean, _deviation;
			DataType	_count;

		public:
			Deviation() : _mean(0), _deviation(0), _count(0)
			{}

			template<typename Callback>
			void Process(DataType val, const Callback& cb)
			{
				DataType prev_mean = _mean;
				_mean = _mean + (val - _mean) / ++_count;
				_deviation = _deviation + (val - prev_mean) * (val - _mean);
				cb(val);
			}

			DataType Get() const	{ return _count != 0 ? _deviation / _count : 0; }
			void Reset()			{ _count = _mean = _deviation = 0; }
		};
	};

	/** @} */

}


#endif

