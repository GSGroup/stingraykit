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


	struct IDataSource
	{
	private:
		template < typename ProcessFunctorType, typename EodFunctorType >
		class FunctorConsumer final : public virtual IDataConsumer
		{
		private:
			ProcessFunctorType				_processFunc;
			EodFunctorType					_eodFunc;

		public:
			template < typename ProcessFunctorType_, typename EodFunctorType_ >
			FunctorConsumer(ProcessFunctorType_&& processFunc, EodFunctorType_&& eodFunc)
				: _processFunc(std::forward<ProcessFunctorType_>(processFunc)), _eodFunc(std::forward<EodFunctorType_>(eodFunc))
			{ }

			size_t Process(ConstByteData data, const ICancellationToken& token) override	{ return _processFunc(data, token); }
			void EndOfData(const ICancellationToken& token) override						{ _eodFunc(token); }
		};

	public:
		virtual ~IDataSource() { }

		virtual void Read(IDataConsumer& consumer, const ICancellationToken& token) = 0;

		template < typename ProcessFunctorType >
		void ReadToFunction(ProcessFunctorType&& processFunc, const ICancellationToken& token)
		{ ReadToFunction(std::forward<ProcessFunctorType>(processFunc), &DefaultEndOfData, token); }

		template < typename ProcessFunctorType, typename EndOfDataFunctorType >
		void ReadToFunction(ProcessFunctorType&& processFunc, EndOfDataFunctorType&& eodFunc, const ICancellationToken& token)
		{
			FunctorConsumer<typename Decay<ProcessFunctorType>::ValueT, typename Decay<EndOfDataFunctorType>::ValueT> consumer(std::forward<ProcessFunctorType>(processFunc), std::forward<EndOfDataFunctorType>(eodFunc));
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


	template < typename MetadataType >
	struct IPacketSource
	{
	private:
		template < typename ProcessFunctorType, typename EodFunctorType >
		class FunctorConsumer final : public virtual IPacketConsumer<MetadataType>
		{
		private:
			ProcessFunctorType				_processFunc;
			EodFunctorType					_eodFunc;

		public:
			template < typename ProcessFunctorType_, typename EodFunctorType_ >
			FunctorConsumer(ProcessFunctorType_&& processFunc, EodFunctorType_&& eodFunc)
				: _processFunc(std::forward<ProcessFunctorType_>(processFunc)), _eodFunc(std::forward<EodFunctorType_>(eodFunc))
			{ }

			bool Process(const Packet<MetadataType>& packet, const ICancellationToken& token) override	{ return _processFunc(packet, token); }
			void EndOfData() override																	{ _eodFunc(); }
		};

	public:
		virtual ~IPacketSource() { }

		virtual void Read(IPacketConsumer<MetadataType>& consumer, const ICancellationToken& token) = 0;

		template < typename ProcessFunctorType >
		void ReadToFunction(ProcessFunctorType&& processFunc, const ICancellationToken& token)
		{ ReadToFunction(std::forward<ProcessFunctorType>(processFunc), &DefaultEndOfData, token); }

		template < typename ProcessFunctorType, typename EndOfDataFunctorType >
		void ReadToFunction(ProcessFunctorType&& processFunc, EndOfDataFunctorType&& eodFunc, const ICancellationToken& token)
		{
			FunctorConsumer<typename Decay<ProcessFunctorType>::ValueT, typename Decay<EndOfDataFunctorType>::ValueT> consumer(std::forward<ProcessFunctorType>(processFunc), std::forward<EndOfDataFunctorType>(eodFunc));
			Read(consumer, token);
		}

	private:
		static void DefaultEndOfData()
		{ STINGRAYKIT_THROW(NotImplementedException()); }
	};


	template < typename MetadataType >
	struct IPacketMediator : public virtual IPacketConsumer<MetadataType>, public virtual IPacketSource<MetadataType>
	{ };


	template < typename MetadataType >
	struct IPacketBuffer : public virtual IPacketMediator<MetadataType>
	{
		using OnOverflowSignature = void (size_t);

		virtual size_t GetDataSize() const = 0;
		virtual size_t GetFreeSize() const = 0;
		virtual size_t GetStorageSize() const = 0;

		virtual bool HasEndOfDataOrException() const = 0;
		virtual optional<MetadataType> WaitForPacket(const ICancellationToken& token) = 0;

		virtual void SetException(const std::exception& ex, const ICancellationToken& token) = 0;
		virtual void Clear() = 0;

		virtual signal_connector<OnOverflowSignature> OnOverflow() const = 0;
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
