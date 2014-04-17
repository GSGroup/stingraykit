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

		virtual size_t Process(ConstByteData data, const CancellationToken& token) = 0;
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

		virtual size_t Process(ConstByteData data, const CancellationToken& token)	{ return _func(data, token); }
		virtual void EndOfData()													{ _func(null, CancellationToken()); }
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


	struct IDataBuffer : public virtual IDataConsumer, public virtual IDataSource
	{ };
	TOOLKIT_DECLARE_PTR(IDataBuffer);


	struct DataInterceptor : public virtual IDataSource
	{
		typedef function<void(optional<ConstByteData>, const CancellationToken&)> FunctionType;

	private:
		IDataSourcePtr	_source;
		FunctionType	_func;

	public:
		DataInterceptor(const IDataSourcePtr& source, const FunctionType& func) :
			_source(source), _func(func)
		{}

		virtual void Read(IDataConsumer& consumer, const CancellationToken& token)
		{ _source->ReadToFunction(bind(&DataInterceptor::DoPush, this, ref(consumer), _1, _2), token); }

	private:
		size_t DoPush(IDataConsumer& consumer, optional<ConstByteData> data, const CancellationToken& token)
		{
			if (data)
			{
				size_t size = consumer.Process(*data, token);
				_func(ConstByteData(*data, 0, size), token);
				return size;
			}

			consumer.EndOfData();
			_func(null, token);
			return 0;
		}
	};
	TOOLKIT_DECLARE_PTR(DataInterceptor);

}

#endif
