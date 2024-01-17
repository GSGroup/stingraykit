#ifndef STINGRAYKIT_IO_IDATASOURCE_H
#define STINGRAYKIT_IO_IDATASOURCE_H

// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/ByteData.h>
#include <stingraykit/signal/signal_connector.h>
#include <stingraykit/thread/atomic.h>

namespace stingray
{

	struct IDataConsumer
	{
		virtual ~IDataConsumer() { }

		virtual size_t Process(ConstByteData data, const ICancellationToken& token) = 0;
		virtual void EndOfData(const ICancellationToken& token) = 0;
	};
	STINGRAYKIT_DECLARE_PTR(IDataConsumer);


	struct InterceptingDataConsumer : public virtual IDataConsumer
	{
		typedef function<void (ConstByteData, const ICancellationToken&)> ProcessFunctionType;
		typedef function<void (const ICancellationToken&)> EodFunctionType;

	private:
		IDataConsumer&			_consumer;
		ProcessFunctionType		_processFunc;
		EodFunctionType			_eodFunc;

	public:
		InterceptingDataConsumer(IDataConsumer& consumer, const ProcessFunctionType& processFunc, const EodFunctionType& eodFunc)
			: _consumer(consumer), _processFunc(processFunc), _eodFunc(eodFunc)
		{ }

		virtual size_t Process(ConstByteData data, const ICancellationToken& token)
		{
			const size_t size = _consumer.Process(data, token);
			_processFunc(ConstByteData(data, 0, size), token);
			return size;
		}

		virtual void EndOfData(const ICancellationToken& token)
		{
			_consumer.EndOfData(token);
			_eodFunc(token);
		}
	};


	template < typename ProcessFunctorType, typename EodFunctorType >
	struct FunctorDataConsumer : public virtual IDataConsumer
	{
	private:
		ProcessFunctorType		_processFunc;
		EodFunctorType			_eodFunc;

	public:
		FunctorDataConsumer(const ProcessFunctorType& processFunc, const EodFunctorType& eodFunc) : _processFunc(processFunc), _eodFunc(eodFunc)
		{ }

		virtual size_t Process(ConstByteData data, const ICancellationToken& token)	{ return _processFunc(data, token); }
		virtual void EndOfData(const ICancellationToken& token)						{ _eodFunc(token); }
	};


	struct IDataSource
	{
		virtual ~IDataSource() { }

		virtual void Read(IDataConsumer& consumer, const ICancellationToken& token) = 0;

		template < typename ProcessFunctorType >
		void ReadToFunction(const ProcessFunctorType& processFunc, const ICancellationToken& token)
		{ ReadToFunction(processFunc, &DefaultEndOfData, token); }

		template < typename ProcessFunctorType, typename EndOfDataFunctorType >
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


	struct IDataMediator : public virtual IDataConsumer, public virtual IDataSource
	{ };
	STINGRAYKIT_DECLARE_PTR(IDataMediator);


	struct IDataBuffer : public virtual IDataMediator
	{
		virtual size_t GetDataSize() const = 0;
		virtual size_t GetFreeSize() const = 0;
		virtual size_t GetStorageSize() const = 0;

		virtual bool HasEndOfDataOrException() const = 0;
		virtual void WaitForData(size_t threshold, const ICancellationToken& token) = 0;

		virtual void SetException(const std::exception& ex, const ICancellationToken& token) = 0;
		virtual void Clear() = 0;

		virtual signal_connector<void (size_t)> OnOverflow() const = 0;
	};
	STINGRAYKIT_DECLARE_PTR(IDataBuffer);


	struct InterceptingDataSource : public virtual IDataSource
	{
		typedef function<void (ConstByteData, const ICancellationToken&)> ProcessFunctionType;
		typedef function<void (const ICancellationToken&)> EodFunctionType;

	private:
		IDataSourcePtr			_source;
		ProcessFunctionType		_processFunc;
		EodFunctionType			_eodFunc;

	public:
		InterceptingDataSource(const IDataSourcePtr& source, const ProcessFunctionType& processFunc, const EodFunctionType& eodFunc)
			: _source(STINGRAYKIT_REQUIRE_NOT_NULL(source)), _processFunc(processFunc), _eodFunc(eodFunc)
		{ }

		virtual void Read(IDataConsumer& c, const ICancellationToken& token)
		{ _source->ReadToFunction(Bind(&InterceptingDataSource::DoPush, this, wrap_ref(c), _1, _2), Bind(&InterceptingDataSource::EndOfData, this, wrap_ref(c), _1), token); }

	private:
		size_t DoPush(IDataConsumer& consumer, ConstByteData data, const ICancellationToken& token)
		{
			const size_t size = consumer.Process(data, token);
			_processFunc(ConstByteData(data, 0, size), token);
			return size;
		}

		void EndOfData(IDataConsumer& consumer, const ICancellationToken& token)
		{
			consumer.EndOfData(token);
			_eodFunc(token);
		}
	};
	STINGRAYKIT_DECLARE_PTR(InterceptingDataSource);


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
			{
				_endOfData = true;
				_consumer.EndOfData(token);
			}
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


	class EmptyDataSource : public virtual IDataSource
	{
	public:
		virtual void Read(IDataConsumer& consumer, const ICancellationToken& token)
		{ consumer.EndOfData(token); }
	};
	STINGRAYKIT_DECLARE_PTR(EmptyDataSource);


	template < typename MetadataType >
	class Packet
	{
	private:
		ConstByteData	_data;
		MetadataType	_metadata;

	public:
		explicit Packet(ConstByteData data, const MetadataType& metadata = MetadataType())
			: _data(data), _metadata(metadata)
		{ }

		ConstByteData GetData() const		{ return _data; }
		size_t GetSize() const				{ return _data.size(); }
		MetadataType GetMetadata() const	{ return _metadata; }
	};


	template < typename MetadataType >
	struct IPacketConsumer
	{
		virtual ~IPacketConsumer() { }

		virtual bool Process(const Packet<MetadataType>& packet, const ICancellationToken& token) = 0;
		virtual void EndOfData() = 0;
	};


	template < typename MetadataType, typename ProcessFunctorType, typename EodFunctorType >
	struct FunctorPacketConsumer : public virtual IPacketConsumer<MetadataType>
	{
	private:
		ProcessFunctorType		_processFunc;
		EodFunctorType			_eodFunc;

	public:
		FunctorPacketConsumer(const ProcessFunctorType& processFunc, const EodFunctorType& eodFunc) : _processFunc(processFunc), _eodFunc(eodFunc)
		{ }

		virtual bool Process(const Packet<MetadataType>& packet, const ICancellationToken& token)	{ return _processFunc(packet, token); }
		virtual void EndOfData()																	{ _eodFunc(); }
	};


	template < typename MetadataType >
	struct IPacketSource
	{
		virtual ~IPacketSource() { }

		virtual void Read(IPacketConsumer<MetadataType>& consumer, const ICancellationToken& token) = 0;

		template < typename ProcessFunctorType >
		void ReadToFunction(const ProcessFunctorType& processFunc, const ICancellationToken& token)
		{ ReadToFunction(processFunc, &DefaultEndOfData, token); }

		template < typename ProcessFunctorType, typename EndOfDataFunctorType >
		void ReadToFunction(const ProcessFunctorType& processFunc, const EndOfDataFunctorType& eodFunc, const ICancellationToken& token)
		{
			FunctorPacketConsumer<MetadataType, ProcessFunctorType, EndOfDataFunctorType> consumer(processFunc, eodFunc);
			Read(consumer, token);
		}

	private:
		static void DefaultEndOfData()
		{ STINGRAYKIT_THROW(NotImplementedException()); }
	};


	inline size_t ConsumeAll(IDataConsumer& consumer, ConstByteData data, const ICancellationToken& token)
	{
		size_t offset = 0;
		while (token && offset < data.size())
			offset += consumer.Process(ConstByteData(data, offset), token);

		return offset;
	}


	template < typename Metadata >
	inline bool ConsumeAll(IPacketConsumer<Metadata>& consumer, const Packet<Metadata>& packet, const ICancellationToken& token)
	{
		bool processed = false;
		while (token && !processed)
			processed = consumer.Process(packet, token);

		return processed;
	}


	class DataConsumeAll : public function_info<size_t (ConstByteData, const ICancellationToken&)>
	{
	private:
		IDataConsumer& _consumer;

	public:
		explicit DataConsumeAll(IDataConsumer& consumer) : _consumer(consumer) { }

		size_t operator () (ConstByteData data, const ICancellationToken& token) const
		{ return ConsumeAll(_consumer, data, token); }
	};


	template < typename Metadata >
	struct PacketConsumeAll : public function_info<bool (const Packet<Metadata>&, const ICancellationToken&)>
	{
	private:
		IPacketConsumer<Metadata>& _consumer;

	public:
		explicit PacketConsumeAll(IPacketConsumer<Metadata>& consumer) : _consumer(consumer) { }

		bool operator () (const Packet<Metadata>& packet, const ICancellationToken& token) const
		{ return ConsumeAll(_consumer, packet, token); }
	};

}

#endif
