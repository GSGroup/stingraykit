// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/io/DataSourceReader.h>


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

			virtual void EndOfData(const ICancellationToken&) { }

			ByteArray GetData() const { return _data; }
		};

		DataStorage storage;
		ReactiveDataSource(_source).Read(storage, token);
		return storage.GetData();
	}

}
