#ifndef __GS_STINGRAY_TOOLKIT_PROGRESSREPORTER_H__
#define __GS_STINGRAY_TOOLKIT_PROGRESSREPORTER_H__


#include <stingray/log/Logger.h>
#include <stingray/toolkit/function.h>
#include <stingray/toolkit/ProgressValue.h>


namespace stingray
{

	class ProgressReporter
	{
		typedef function<void (ProgressValue)>			ReporterFunc;
		typedef function<ProgressValue (ProgressValue)>	TransformerFunc;

	private:
		ReporterFunc		_reporter;
		TransformerFunc		_transformer;

	public:
		explicit ProgressReporter(const ReporterFunc& reporter = &ProgressReporter::DummyReporter, const TransformerFunc& transformer = &ProgressReporter::DummyTransformer)
			: _reporter(reporter), _transformer(transformer)
		{ }

		ProgressReporter(const ProgressReporter& other, const TransformerFunc& transformer)
			: _reporter(other._reporter), _transformer(transformer)
		{ }

		void operator()(size_t current, size_t total) const
		{
			try
			{ _reporter(_transformer(ProgressValue(current, total))); }
			catch (const std::exception& ex)
			{ Logger().Error() << "Could not report progress:\n" << diagnostic_information(ex); }
		}

	private:
		static void DummyReporter(const ProgressValue& progress) { }
		static ProgressValue DummyTransformer(const ProgressValue& progress) { return progress; }
	};

}


#endif
