#ifndef STINGRAYKIT_ACCUMULATORS_H
#define STINGRAYKIT_ACCUMULATORS_H

// Copyright (c) 2011 - 2022, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

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

