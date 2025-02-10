#ifndef STINGRAYKIT_IO_IDATASOURCE_H
#define STINGRAYKIT_IO_IDATASOURCE_H

// Copyright (c) 2011 - 2025, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/ByteData.h>
#include <stingraykit/signal/signal_connector.h>

namespace stingray
{

	struct IDataConsumer
	{
		virtual ~IDataConsumer() { }

		virtual size_t Process(ConstByteData data, const ICancellationToken& token) = 0;
		virtual void EndOfData(const ICancellationToken& token) = 0;
	};
	STINGRAYKIT_DECLARE_PTR(IDataConsumer);


	class InterceptingDataConsumer final : public virtual IDataConsumer
	{
		using ProcessFunctionType = function<void (ConstByteData, const ICancellationToken&)>;
		using EodFunctionType = function<void (const ICancellationToken&)>;

	private:
		IDataConsumer&					_consumer;
		ProcessFunctionType				_processFunc;
		EodFunctionType					_eodFunc;

	public:
		InterceptingDataConsumer(IDataConsumer& consumer, const ProcessFunctionType& processFunc, const EodFunctionType& eodFunc)
			: _consumer(consumer), _processFunc(processFunc), _eodFunc(eodFunc)
		{ }

		size_t Process(ConstByteData data, const ICancellationToken& token) override
		{
			const size_t size = _consumer.Process(data, token);
			_processFunc(ConstByteData(data, 0, size), token);
			return size;
		}

		void EndOfData(const ICancellationToken& token) override
		{
			_consumer.EndOfData(token);
			_eodFunc(token);
		}
	};


	template < typename ProcessFunctorType, typename EodFunctorType >
	class FunctorDataConsumer final : public virtual IDataConsumer
	{
	private:
		ProcessFunctorType				_processFunc;
		EodFunctorType					_eodFunc;

	public:
		FunctorDataConsumer(const ProcessFunctorType& processFunc, const EodFunctorType& eodFunc) : _processFunc(processFunc), _eodFunc(eodFunc)
		{ }

		size_t Process(ConstByteData data, const ICancellationToken& token) override	{ return _processFunc(data, token); }
		void EndOfData(const ICancellationToken& token) override						{ _eodFunc(token); }
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
		using OnOverflowSignature = void (size_t);

		virtual size_t GetDataSize() const = 0;
		virtual size_t GetFreeSize() const = 0;
		virtual size_t GetStorageSize() const = 0;

		virtual bool HasEndOfDataOrException() const = 0;
		virtual void WaitForData(size_t threshold, const ICancellationToken& token) = 0;

		virtual void SetException(const std::exception& ex, const ICancellationToken& token) = 0;
		virtual void Clear() = 0;

		virtual signal_connector<OnOverflowSignature> OnOverflow() const = 0;
	};
	STINGRAYKIT_DECLARE_PTR(IDataBuffer);


	class InterceptingDataSource final : public virtual IDataSource
	{
		using ProcessFunctionType = function<void (ConstByteData, const ICancellationToken&)>;
		using EodFunctionType = function<void (const ICancellationToken&)>;

	private:
		IDataSourcePtr					_source;
		ProcessFunctionType				_processFunc;
		EodFunctionType					_eodFunc;

	public:
		InterceptingDataSource(const IDataSourcePtr& source, const ProcessFunctionType& processFunc, const EodFunctionType& eodFunc)
			: _source(STINGRAYKIT_REQUIRE_NOT_NULL(source)), _processFunc(processFunc), _eodFunc(eodFunc)
		{ }

		void Read(IDataConsumer& c, const ICancellationToken& token) override
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


	class ReactiveDataSource final : public virtual IDataSource
	{
		class Consumer final : public virtual IDataConsumer
		{
		private:
			IDataConsumer&				_consumer;
			atomic<bool>				_endOfData;

		public:
			explicit Consumer(IDataConsumer& consumer)
				: _consumer(consumer), _endOfData(false)
			{ }

			bool IsEndOfData() const { return _endOfData; }

			size_t Process(ConstByteData data, const ICancellationToken& token) override { return _consumer.Process(data, token); }

			void EndOfData(const ICancellationToken& token) override
			{
				_endOfData = true;
				_consumer.EndOfData(token);
			}
		};

	private:
		IDataSourcePtr					_source;

	public:
		explicit ReactiveDataSource(const IDataSourcePtr& source)
			: _source(source)
		{ }

		void Read(IDataConsumer& consumer, const ICancellationToken& token) override
		{
			Consumer reactiveConsumer(consumer);
			while (token && !reactiveConsumer.IsEndOfData())
				_source->Read(reactiveConsumer, token);
		}
	};
	STINGRAYKIT_DECLARE_PTR(ReactiveDataSource);


	class EmptyDataSource final : public virtual IDataSource
	{
	public:
		void Read(IDataConsumer& consumer, const ICancellationToken& token) override
		{ consumer.EndOfData(token); }
	};
	STINGRAYKIT_DECLARE_PTR(EmptyDataSource);


	template < typename MetadataType >
	class Packet
	{
	private:
		ConstByteData					_data;
		MetadataType					_metadata;

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
	class FunctorPacketConsumer final : public virtual IPacketConsumer<MetadataType>
	{
	private:
		ProcessFunctorType				_processFunc;
		EodFunctorType					_eodFunc;

	public:
		FunctorPacketConsumer(const ProcessFunctorType& processFunc, const EodFunctorType& eodFunc) : _processFunc(processFunc), _eodFunc(eodFunc)
		{ }

		bool Process(const Packet<MetadataType>& packet, const ICancellationToken& token) override	{ return _processFunc(packet, token); }
		void EndOfData() override																	{ _eodFunc(); }
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
		IDataConsumer&					_consumer;

	public:
		explicit DataConsumeAll(IDataConsumer& consumer) : _consumer(consumer) { }

		size_t operator () (ConstByteData data, const ICancellationToken& token) const
		{ return ConsumeAll(_consumer, data, token); }
	};


	template < typename Metadata >
	class PacketConsumeAll : public function_info<bool (const Packet<Metadata>&, const ICancellationToken&)>
	{
	private:
		IPacketConsumer<Metadata>&		_consumer;

	public:
		explicit PacketConsumeAll(IPacketConsumer<Metadata>& consumer) : _consumer(consumer) { }

		bool operator () (const Packet<Metadata>& packet, const ICancellationToken& token) const
		{ return ConsumeAll(_consumer, packet, token); }
	};

}

#endif
