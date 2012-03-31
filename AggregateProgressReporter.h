#ifndef __GS_STINGRAY_TOOLKIT_AGGREGATEPROGRESSREPORTER_H__
#define __GS_STINGRAY_TOOLKIT_AGGREGATEPROGRESSREPORTER_H__


#include <stingray/toolkit/function.h>
#include <stingray/toolkit/ProgressValue.h>


namespace stingray
{

	class AggregateProgressReporter
	{
		typedef function<void(const ProgressValue&)> ProgressReporterFunc;

	private:
		ProgressReporterFunc	_progressReporter;
		size_t 					_actionIndex;
		size_t 					_actionCount;

	public:
		AggregateProgressReporter(const ProgressReporterFunc& progressReporter, size_t actionIndex, size_t actionCount)
			: _progressReporter(progressReporter), _actionIndex(actionIndex), _actionCount(actionCount)
		{ TOOLKIT_INDEX_CHECK(_actionIndex < _actionCount); }

		void operator()(const ProgressValue& actionProgress) const
		{ _progressReporter(ProgressValue(_actionCount != 0 ? (_actionIndex * 100 + actionProgress.InPercents()) / _actionCount : 0, 100)); }
	};

}


#endif
