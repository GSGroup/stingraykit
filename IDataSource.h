#ifndef STINGRAY_TOOLKIT_IDATASOURCE_H
#define STINGRAY_TOOLKIT_IDATASOURCE_H


#include <stingray/threads/CancellationToken.h>
#include <stingray/toolkit/reference.h>
#include <stingray/toolkit/bind.h>

namespace stingray
{

	struct IDataConsumer
	{
		virtual ~IDataConsumer() {}

		virtual size_t Process(ConstByteData data) = 0;
		virtual void EndOfData() = 0;
	};
	TOOLKIT_DECLARE_PTR(IDataConsumer);


	template <typename FunctorType>
	struct FunctorDataConsumer : public virtual IDataConsumer
	{
	private:
		FunctorType					_func;

	public:
		FunctorDataConsumer(const FunctorType& func) : _func(func)
		{}

		virtual size_t Process(ConstByteData data)	{ return _func(data); }
		virtual void EndOfData()					{ _func(null); }
	};


	struct IDataSource
	{
		virtual ~IDataSource() {}

		virtual void Read(IDataConsumer& consumer, const CancellationToken& token) = 0;

		template <typename FunctorType>
		void ReadToFunction(const FunctorType& func, const CancellationToken& token)
		{
			FunctorDataConsumer<FunctorType> consumer(func);
			Read(consumer, token);
		}
	};
	TOOLKIT_DECLARE_PTR(IDataSource);

	struct DataInterceptor : public virtual IDataSource
	{
		typedef function<void(optional<ConstByteData>)> FunctionType;

	private:
		IDataSourcePtr	_source;
		FunctionType	_func;

	public:
		DataInterceptor(const IDataSourcePtr& source, const FunctionType& func) :
			_source(source), _func(func)
		{}

		virtual void Read(IDataConsumer& consumer, const CancellationToken& token)
		{ _source->ReadToFunction(bind(&DataInterceptor::DoPush, this, ref(consumer), _1), token); }

	private:
		size_t DoPush(IDataConsumer& consumer, optional<ConstByteData> data)
		{
			if (data)
			{
				size_t size = consumer.Process(*data);
				_func(ConstByteData(*data, 0, size));
				return size;
			}

			consumer.EndOfData();
			_func(null);
			return 0;
		}
	};
	TOOLKIT_DECLARE_PTR(DataInterceptor);

}

#endif
