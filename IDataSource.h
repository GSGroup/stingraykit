#ifndef STINGRAY_TOOLKIT_IDATASOURCE_H
#define STINGRAY_TOOLKIT_IDATASOURCE_H

namespace stingray
{

	struct IDataConsumer
	{
		virtual ~IDataConsumer() {}

		virtual size_t Process(ConstByteData data) = 0;
		virtual void EndOfData() = 0;
	};


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

}

#endif
