#ifndef STINGRAYKIT_IO_IDATASOURCE_H
#define STINGRAYKIT_IO_IDATASOURCE_H

// Copyright (c) 2011 - 2015, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <stingraykit/collection/ByteData.h>
#include <stingraykit/function/bind.h>
#include <stingraykit/reference.h>
#include <stingraykit/thread/ICancellationToken.h>
#include <stingraykit/thread/atomic.h>

namespace stingray
{

	struct IDataConsumer
	{
		virtual ~IDataConsumer() {}

		virtual size_t Process(ConstByteData data, const ICancellationToken& token) = 0;
		virtual void EndOfData(const ICancellationToken& token) = 0;
	};
	STINGRAYKIT_DECLARE_PTR(IDataConsumer);


	template <typename ProcessFunctorType, typename EodFunctorType>
	struct FunctorDataConsumer : public virtual IDataConsumer
	{
	private:
		ProcessFunctorType		_processFunc;
		EodFunctorType			_eodFunc;

	public:
		FunctorDataConsumer(const ProcessFunctorType& processFunc, const EodFunctorType& eodFunc) : _processFunc(processFunc), _eodFunc(eodFunc)
		{}

		virtual size_t Process(ConstByteData data, const ICancellationToken& token)	{ return _processFunc(data, token); }
		virtual void EndOfData(const ICancellationToken& token)						{ _eodFunc(token); }
	};


	struct IDataSource
	{
		virtual ~IDataSource() {}

		virtual void Read(IDataConsumer& consumer, const ICancellationToken& token) = 0;

		template <typename ProcessFunctorType>
		void ReadToFunction(const ProcessFunctorType& processFunc, const ICancellationToken& token)
		{ ReadToFunction(processFunc, &DefaultEndOfData, token); }

		template <typename ProcessFunctorType, typename EndOfDataFunctorType>
		void ReadToFunction(const ProcessFunctorType& processFunc, const EndOfDataFunctorType& eodFunc, const ICancellationToken& token)
		{
			FunctorDataConsumer<ProcessFunctorType, EndOfDataFunctorType> consumer(processFunc, eodFunc);
			Read(consumer, token);
		}

	private:
		static void DefaultEndOfData(const ICancellationToken&)
		{ STINGRAYKIT_THROW(NotImplementedException()); }
	};
	STINGRAYKIT_DECLARE_PTR(IDataSource);


	struct IDataBuffer : public virtual IDataConsumer, public virtual IDataSource
	{ };
	STINGRAYKIT_DECLARE_PTR(IDataBuffer);


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

		virtual void Read(IDataConsumer& c, const ICancellationToken& token)
		{ _source->ReadToFunction(bind(&DataInterceptor::DoPush, this, ref(c), _1, _2), bind(&DataInterceptor::Eod, this, ref(c), _1), token); }

	private:
		size_t DoPush(IDataConsumer& consumer, ConstByteData data, const ICancellationToken& token)
		{
			size_t size = consumer.Process(data, token);
			_func(ConstByteData(data, 0, size));
			return size;
		}

		void Eod(IDataConsumer& consumer, const ICancellationToken& token)
		{ consumer.EndOfData(token); _eod(); }
	};
	STINGRAYKIT_DECLARE_PTR(DataInterceptor);


	class ReactiveDataSource : public virtual IDataSource
	{

		class ReactiveDataConsumer : public virtual IDataConsumer
		{
		private:
			IDataConsumer&	_consumer;
			atomic<bool>	_endOfData;

		public:
			explicit ReactiveDataConsumer(IDataConsumer& consumer)
				: _consumer(consumer), _endOfData(false)
			{ }

			bool IsEndOfData() const { return _endOfData; }

			virtual size_t Process(ConstByteData data, const ICancellationToken& token) { return _consumer.Process(data, token); }

			virtual void EndOfData(const ICancellationToken& token)
			{ _endOfData = true; _consumer.EndOfData(token); }
		};

	private:
		IDataSourcePtr	_source;

	public:
		explicit ReactiveDataSource(const IDataSourcePtr& source)
			: _source(source)
		{ }

		virtual void Read(IDataConsumer& consumer, const ICancellationToken& token)
		{
			ReactiveDataConsumer reactiveConsumer(consumer);
			while (token && !reactiveConsumer.IsEndOfData())
				_source->Read(reactiveConsumer, token);
		}
	};
	STINGRAYKIT_DECLARE_PTR(ReactiveDataSource);


	template<typename MetadataType>
	class Packet
	{
	private:
		ConstByteData	_data;
		MetadataType	_metadata;

	public:
		explicit Packet(ConstByteData data, const MetadataType& metadata = MetadataType()) :
			_data(data), _metadata(metadata)
		{ }

		ConstByteData GetData() const		{ return _data; }
		size_t GetSize() const				{ return _data.size(); }
		MetadataType GetMetadata() const	{ return _metadata; }
	};


	template<typename MetadataType>
	struct IPacketConsumer
	{
		virtual ~IPacketConsumer() {}

		virtual bool Process(const Packet<MetadataType>& packet, const ICancellationToken& token) = 0;
		virtual void EndOfData() = 0;
	};


	template <typename MetadataType, typename ProcessFunctorType, typename EodFunctorType>
	struct FunctorPacketConsumer : public virtual IPacketConsumer<MetadataType>
	{
	private:
		ProcessFunctorType		_processFunc;
		EodFunctorType			_eodFunc;

	public:
		FunctorPacketConsumer(const ProcessFunctorType& processFunc, const EodFunctorType& eodFunc) : _processFunc(processFunc), _eodFunc(eodFunc)
		{}

		virtual bool Process(const Packet<MetadataType>& packet, const ICancellationToken& token)	{ return _processFunc(packet, token); }
		virtual void EndOfData()																	{ _eodFunc(); }
	};


	template<typename MetadataType>
	struct IPacketSource
	{
		virtual ~IPacketSource() {}

		virtual void Read(IPacketConsumer<MetadataType>& consumer, const ICancellationToken& token) = 0;

		template <typename ProcessFunctorType>
		void ReadToFunction(const ProcessFunctorType& processFunc, const ICancellationToken& token)
		{ ReadToFunction(processFunc, &DefaultEndOfData, token); }

		template <typename ProcessFunctorType, typename EndOfDataFunctorType>
		void ReadToFunction(const ProcessFunctorType& processFunc, const EndOfDataFunctorType& eodFunc, const ICancellationToken& token)
		{
			FunctorPacketConsumer<MetadataType, ProcessFunctorType, EndOfDataFunctorType> consumer(processFunc, eodFunc);
			Read(consumer, token);
		}

	private:
		static void DefaultEndOfData()
		{ STINGRAYKIT_THROW(NotImplementedException()); }
	};


	struct ByteDataPacketSource : public virtual IPacketSource<EmptyType>
	{
	private:
		optional<ConstByteData> _data;

	public:
		ByteDataPacketSource()
		{}

		ByteDataPacketSource(ConstByteData data)
			: _data(data)
		{ }

		void SetData(ConstByteData data)
		{ _data = data; }

		virtual void Read(IPacketConsumer<EmptyType>& consumer, const ICancellationToken& token)
		{
			if (_data)
			{
				consumer.Process(Packet<EmptyType>(*_data), token);
				_data.reset();
			}
		}
	};
	STINGRAYKIT_DECLARE_PTR(ByteDataPacketSource);


	inline size_t ConsumeAll(IDataConsumer& consumer, ConstByteData data, const ICancellationToken& token)
	{
		size_t offset = 0;
		while (token && offset < data.size())
			offset += consumer.Process(ConstByteData(data, offset), token);

		return offset;
	}


	template <typename Metadata>
	inline bool ConsumeAll(IPacketConsumer<Metadata>& consumer, Packet<Metadata> packet, const ICancellationToken& token)
	{
		bool processed = false;
		while (token && !processed)
			processed = consumer.Process(packet, token);

		return processed;
	}


}

#endif
