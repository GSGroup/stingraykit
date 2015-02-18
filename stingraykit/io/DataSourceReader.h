#ifndef STINGRAYKIT_IO_DATASOURCEREADER_H
#define STINGRAYKIT_IO_DATASOURCEREADER_H


#include <stingraykit/io/IDataSource.h>
#include <stingraykit/thread/DummyCancellationToken.h>


namespace stingray
{

	class DataSourceReader
	{
	private:
		IDataSourcePtr	_source;

	public:
		explicit DataSourceReader(const IDataSourcePtr& source);

		ByteArray ReadToEnd(const ICancellationToken& token = DummyCancellationToken());
	};

}


#endif
