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


	template <typename ProcessFunctorType, typename EodFunctorType>
	struct FunctorDataConsumer : public virtual IDataConsumer
	{
	private:
		ProcessFunctorType		_processFunc;
		EodFunctorType			_eodFunc;

	public:
		FunctorDataConsumer(const ProcessFunctorType& processFunc, const EodFunctorType& eodFunc) : _processFunc(processFunc), _eodFunc(eodFunc)
		{}

		virtual size_t Process(ConstByteData data, const CancellationToken& token)	{ return _processFunc(data, token); }
		virtual void EndOfData()													{ _eodFunc(); }
	};


	struct IDataSource
	{
		virtual ~IDataSource() {}

		virtual void Read(IDataConsumer& consumer, const CancellationToken& token) = 0;

		template <typename ProcessFunctorType>
		void ReadToFunction(const ProcessFunctorType& processFunc, const CancellationToken& token)
		{ ReadToFunction(processFunc, &DefaultEndOfData, token); }

		template <typename ProcessFunctorType, typename EndOfDataFunctorType>
		void ReadToFunction(const ProcessFunctorType& processFunc, const EndOfDataFunctorType& eodFunc, const CancellationToken& token)
		{
			FunctorDataConsumer<ProcessFunctorType, EndOfDataFunctorType> consumer(processFunc, eodFunc);
			Read(consumer, token);
		}

	private:
		static void DefaultEndOfData()
		{ TOOLKIT_THROW(NotImplementedException()); }
	};
	TOOLKIT_DECLARE_PTR(IDataSource);


	struct IDataBuffer : public virtual IDataConsumer, public virtual IDataSource
	{ };
	TOOLKIT_DECLARE_PTR(IDataBuffer);


	struct DataInterceptor : public virtual IDataSource
	{
		typedef function<void(ConstByteData)> FunctionType;
		typedef function<void()> EodFunctionType;

	private:
		IDataSourcePtr	_source;
		FunctionType	_func;
		EodFunctionType	_eod;

	public:
		DataInterceptor(const IDataSourcePtr& source, const FunctionType& func, const EodFunctionType& eod) :
			_source(source), _func(func), _eod(eod)
		{}

		virtual void Read(IDataConsumer& c, const CancellationToken& token)
		{ _source->ReadToFunction(bind(&DataInterceptor::DoPush, this, ref(c), _1, _2), bind(&DataInterceptor::Eod, this, ref(c)), token); }

	private:
		size_t DoPush(IDataConsumer& consumer, ConstByteData data, const CancellationToken& token)
		{
			size_t size = consumer.Process(data, token);
			_func(ConstByteData(data, 0, size));
			return size;
		}

		void Eod(IDataConsumer& consumer)
		{ consumer.EndOfData(); _eod(); }
	};
	TOOLKIT_DECLARE_PTR(DataInterceptor);


	struct IPacketConsumer
	{
		virtual ~IPacketConsumer() {}

		virtual bool Process(ConstByteData packet, const CancellationToken& token) = 0;
		virtual void EndOfData() = 0;
	};
	TOOLKIT_DECLARE_PTR(IPacketConsumer);


	template <typename ProcessFunctorType, typename EodFunctorType>
	struct FunctorPacketConsumer : public virtual IPacketConsumer
	{
	private:
		ProcessFunctorType		_processFunc;
		EodFunctorType			_eodFunc;

	public:
		FunctorPacketConsumer(const ProcessFunctorType& processFunc, const EodFunctorType& eodFunc) : _processFunc(processFunc), _eodFunc(eodFunc)
		{}

		virtual bool Process(ConstByteData data, const CancellationToken& token)	{ return _processFunc(data, token); }
		virtual void EndOfData()													{ _eodFunc(); }
	};


	struct IPacketSource
	{
		virtual ~IPacketSource() {}

		virtual void Read(IPacketConsumer& consumer, const CancellationToken& token) = 0;

		template <typename ProcessFunctorType>
		void ReadToFunction(const ProcessFunctorType& processFunc, const CancellationToken& token)
		{ ReadToFunction(processFunc, &DefaultEndOfData, token); }

		template <typename ProcessFunctorType, typename EndOfDataFunctorType>
		void ReadToFunction(const ProcessFunctorType& processFunc, const EndOfDataFunctorType& eodFunc, const CancellationToken& token)
		{
			FunctorPacketConsumer<ProcessFunctorType, EndOfDataFunctorType> consumer(processFunc, eodFunc);
			Read(consumer, token);
		}

	private:
		static void DefaultEndOfData()
		{ TOOLKIT_THROW(NotImplementedException()); }
	};
	TOOLKIT_DECLARE_PTR(IPacketSource);


	struct ByteDataPacketSource : public virtual IPacketSource
	{
	private:
		optional<ConstByteData> _data;

	public:
		ByteDataPacketSource()
		{}

		void SetData(ConstByteData data)
		{ _data = data; }

		virtual void Read(IPacketConsumer& consumer, const CancellationToken& token)
		{
			if (_data)
			{
				consumer.Process(*_data, token);
				_data.reset();
			}
		}
	};
	TOOLKIT_DECLARE_PTR(ByteDataPacketSource);


}

#endif
