#ifndef STINGRAYKIT_AGGREGATEPROGRESSREPORTER_H
#define STINGRAYKIT_AGGREGATEPROGRESSREPORTER_H

// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <stingraykit/function/function.h>
#include <stingraykit/ProgressValue.h>


namespace stingray
{

	class AggregateProgressReporter : public function_info<void(const ProgressValue&)>
	{
		typedef function<void(const ProgressValue&)> ProgressReporterFunc;

	private:
		ProgressReporterFunc	_progressReporter;
		size_t 					_actionIndex;
		size_t 					_actionSize;
		size_t 					_actionCount;

	public:
		AggregateProgressReporter(const ProgressReporterFunc& progressReporter, size_t actionIndex, size_t actionCount)
			: _progressReporter(progressReporter), _actionIndex(actionIndex), _actionSize(1), _actionCount(actionCount)
		{ STINGRAYKIT_CHECK(_actionIndex < _actionCount, IndexOutOfRangeException(_actionIndex, _actionCount)); }

		AggregateProgressReporter(const ProgressReporterFunc& progressReporter, size_t actionIndex, size_t actionSize, size_t actionCount)
			: _progressReporter(progressReporter), _actionIndex(actionIndex), _actionSize(actionSize), _actionCount(actionCount)
		{ STINGRAYKIT_CHECK((_actionIndex + _actionSize - 1) < _actionCount, IndexOutOfRangeException(_actionIndex + actionSize, _actionCount)); }

		void operator()(const ProgressValue& actionProgress) const
		{ _progressReporter(ProgressValue(_actionCount != 0 ? (_actionIndex * 100 + _actionSize * actionProgress.InPercents()) / _actionCount : 0, 100)); }
	};

}


#endif
