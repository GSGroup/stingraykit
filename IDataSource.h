#ifndef STINGRAY_TOOLKIT_IDATASOURCE_H
#define STINGRAY_TOOLKIT_IDATASOURCE_H

namespace stingray
{

	struct IDataConsumer
	{
		virtual ~IDataConsumer() {}

		virtual size_t Process(ConstByteData data) = 0;
		virtual void EndOfData() = 0;

		virtual const CancellationToken& GetToken() = 0;
	};


	struct IDataSource
	{
		virtual ~IDataSource() {}

		virtual void Read(IDataConsumer& consumer) = 0;
	};
	TOOLKIT_DECLARE_PTR(IDataSource);

}

#endif
