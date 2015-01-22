#include <stingray/stingraykit/stingraykit/io/DataSourceReader.h>


namespace stingray
{

	DataSourceReader::DataSourceReader(const IDataSourcePtr& source)
		: _source(source)
	{ }


	ByteArray DataSourceReader::ReadToEnd(const ICancellationToken& token)
	{
		class DataStorage : public virtual IDataConsumer
		{
		private:
			ByteArray	_data;

		public:
			virtual size_t Process(ConstByteData data, const ICancellationToken&)
			{
				_data.append(data.begin(), data.end());
				return data.size();
			}

			virtual void EndOfData() { }

			ByteArray GetData() const { return _data; }
		};

		DataStorage storage;
		ReactiveDataSource(_source).Read(storage, token);
		return storage.GetData();
	}

}
