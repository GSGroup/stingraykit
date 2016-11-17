#ifndef STINGRAYKIT_IO_CONSUMERPROXY_H
#define STINGRAYKIT_IO_CONSUMERPROXY_H

// Copyright (c) 2011 - 2015, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <stingraykit/io/IDataSource.h>
#include <stingraykit/thread/DummyCancellationToken.h>

namespace stingray
{


	///
	///	@brief The ConsumerDepacketer class turns IDataConsumer into the IPacketConsumer.
	///
	class ConsumerDepacketer : public virtual IPacketConsumer<EmptyType>
	{
	private:
		IDataConsumerPtr	_lifeAssurance;
		IDataConsumer&		_consumer;

	public:
		ConsumerDepacketer(const IDataConsumerPtr& consumer)
			:	_lifeAssurance(consumer),
				_consumer(*_lifeAssurance)
		{}


		ConsumerDepacketer(IDataConsumer& consumer)
			:	_lifeAssurance(null),
				_consumer(consumer)
		{}


		virtual bool Process(const Packet<EmptyType>& packet, const ICancellationToken& token)
		{ return _consumer.Process(packet.GetData(), token); }


		virtual void EndOfData()
		{ _consumer.EndOfData(DummyCancellationToken()); }
	};
	STINGRAYKIT_DECLARE_PTR(ConsumerDepacketer);


	///
	///	@brief The ConsumerPacketer class turns IPacketConsumer into the IDataConsumer.
	///
	class ConsumerPacketer : public virtual IDataConsumer
	{
		typedef IPacketConsumer<EmptyType> PacketConsumer;
		STINGRAYKIT_DECLARE_PTR(PacketConsumer);

	private:
		PacketConsumerPtr	_lifeAssurance;
		PacketConsumer&		_consumer;
		optional<size_t>	_packetSize;

	public:
		ConsumerPacketer(const PacketConsumerPtr& consumer, optional<size_t> packetSize = null)
			:	_lifeAssurance(consumer),
				_consumer(*_lifeAssurance),
				_packetSize(packetSize)
		{}


		ConsumerPacketer(PacketConsumer& consumer, optional<size_t> packetSize = null)
			:	_lifeAssurance(null),
				_consumer(consumer),
				_packetSize(packetSize)
		{}


		virtual size_t Process(ConstByteData data, const ICancellationToken& token)
		{
			ConstByteData packetData(data, 0, (_packetSize ? *_packetSize : data.size()));
			return _consumer.Process(Packet<EmptyType>(packetData), token) ? packetData.size() : 0;
		}


		virtual void EndOfData(const ICancellationToken&)
		{ _consumer.EndOfData(); }
	};
	STINGRAYKIT_DECLARE_PTR(ConsumerPacketer);


	namespace DataFilter
	{


		///
		///	@brief The DataFilter::Consumer class is a IDataConsumer proxy,
		///		which filters incoming data using client-supplied function.
		///
		class Consumer : public virtual IDataConsumer
		{
		public:
			typedef	function<ConstByteData(ConstByteData)>		Filter;

		private:
			IDataConsumerPtr	_lifeAssurance;
			IDataConsumer&		_consumer;
			Filter				_filter;

		public:
			Consumer(const IDataConsumerPtr& consumer, const Filter& filter)
				:	_lifeAssurance(consumer),
					_consumer(*_lifeAssurance),
					_filter(filter)
			{}


			Consumer(IDataConsumer& consumer, const Filter& filter)
				:	_lifeAssurance(null),
					_consumer(consumer),
					_filter(filter)
			{}


			virtual size_t Process(ConstByteData data, const ICancellationToken& token)
			{ ConsumeAll(_consumer, _filter(data), token); return data.size(); }


			virtual void EndOfData(const ICancellationToken& token)
			{ _consumer.EndOfData(token); }
		};
		STINGRAYKIT_DECLARE_PTR(Consumer);


		///
		///	@brief The DataFilter::CutHead class is a filter functor for DataFilter::Consumer,
		///		which cuts the first _head_ bytes from incoming ByteData sequence, like this:
		///		|////////|////----|--------|--------|--------|
		///
		class CutHead : public function_info<ConstByteData(ConstByteData)>
		{
		private:
			size_t	_head;

		public:
			CutHead(size_t head)	:	_head(head)	{}

			ConstByteData operator()(ConstByteData data)
			{
				const size_t offset = std::min(_head, data.size());
				_head -= offset;
				return ConstByteData(data, offset);
			}
		};


		///
		///	@brief The DataFilter::CutTail class is a filter functor for DataFilter::Consumer,
		///		which passes first _head_ bytes from incoming ByteData sequence (and omits the rest of the tail), like this:
		///		|--------|--------|--------|--//////|////////|
		///
		class CutTail : public function_info<ConstByteData(ConstByteData)>
		{
		private:
			size_t	_head;

		public:
			CutTail(size_t head)	:	_head(head)	{}

			ConstByteData operator()(ConstByteData data)
			{
				const size_t size = std::min(_head, data.size());
				_head -= size;
				return ConstByteData(data, 0, size);
			}
		};


		namespace CutHeadRepetitive
		{
			///
			///	@brief The DataFilter:: CutHeadRepetitive::FromHead class is a filter functor for DataFilter::Consumer,
			///		which cuts the first _head_ bytes from *each* incoming ByteData in sequence, like this:
			///		|///-----|///-----|///-----|///-----|
			///
			class FromHead : public function_info<ConstByteData(ConstByteData)>
			{
			private:
				const size_t	_head;

			public:
				FromHead(size_t head)	:	_head(head)	{}

				ConstByteData operator()(ConstByteData data) const
				{ return ConstByteData(data, std::min(_head, data.size())); }
			};

			///
			///	@brief The DataFilter::CutHeadRepetitive::FromTail class is a filter functor for DataFilter::Consumer,
			///		which passes the last _tail_ bytes from *each* incoming ByteData in sequence, like this:
			///		|///-----|///-----|///-----|///-----|
			///
			class FromTail : public function_info<ConstByteData(ConstByteData)>
			{
			private:
				const size_t	_tail;

			public:
				FromTail(size_t tail)	:	_tail(tail)	{}

				ConstByteData operator()(ConstByteData data) const
				{ return ConstByteData(data, data.size() - std::min(_tail, data.size())); }
			};
		}


		namespace CutTailRepetitive
		{
			///
			///	@brief The DataFilter::CutTailRepetitive::FromHead class is a filter functor for DataFilter::Consumer,
			///		which passes the first _head_ bytes from *each* incoming ByteData in sequence, like this:
			///		|-----///|-----///|-----///|-----///|
			///
			class FromHead : public function_info<ConstByteData(ConstByteData)>
			{
			private:
				const size_t	_head;

			public:
				FromHead(size_t head)	:	_head(head)	{}

				ConstByteData operator()(ConstByteData data) const
				{ return ConstByteData(data, 0, std::min(_head, data.size())); }
			};

			///
			///	@brief The DataFilter::CutTailRepetitive::FromTail class is a filter functor for DataFilter::Consumer,
			///		which cuts the last _tail_ bytes from *each* incoming ByteData in sequence, like this:
			///		|-----///|-----///|-----///|-----///|
			///
			class FromTail : public function_info<ConstByteData(ConstByteData)>
			{
			private:
				const size_t	_tail;

			public:
				FromTail(size_t tail)	:	_tail(tail)	{}

				ConstByteData operator()(ConstByteData data) const
				{ return ConstByteData(data, 0, data.size() - std::min(_tail, data.size())); }
			};
		}


	}


}

#endif
