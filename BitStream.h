#ifndef STINGRAY_TOOLKIT_BIT_STREAM_H__
#define STINGRAY_TOOLKIT_BIT_STREAM_H__

#include <iterator>

#include <stingray/toolkit/toolkit.h>
#include <stingray/toolkit/BitsGetter.h>

namespace stingray
{

	namespace Detail
	{
		TOOLKIT_DECLARE_NESTED_TYPE_CHECK(container_type);

		template < typename IteratorType, bool HasContainer = HasNestedType_container_type<IteratorType>::Value >
		struct GetIteratorValueType
		{ typedef typename std::iterator_traits<IteratorType>::value_type ValueT; };

		template < typename IteratorType >
		struct GetIteratorValueType<IteratorType, true>
		{ typedef typename IteratorType::container_type::value_type	ValueT; };

		template < typename ByteDataType_, bool BigEndian, int Size >
		class BasicBitStreamReadProxy
		{
		private:
			ByteDataType_*		_buf;
			size_t				_offset;

		public:
			BasicBitStreamReadProxy(ByteDataType_* buf, size_t offset)
				: _buf(buf), _offset(offset)
			{ }

			template < typename T >
			operator T() const
			{
				BasicBitsGetter<ByteDataType_, BigEndian> bits(*_buf, _offset >> 3);
				switch (_offset & 7)
				{
				case 0:		return bits.template Get<0, Size>();
				case 1:		return bits.template Get<1, Size>();
				case 2:		return bits.template Get<2, Size>();
				case 3:		return bits.template Get<3, Size>();
				case 4:		return bits.template Get<4, Size>();
				case 5:		return bits.template Get<5, Size>();
				case 6:		return bits.template Get<6, Size>();
				case 7:		return bits.template Get<7, Size>();
				default:	TOOLKIT_THROW(std::runtime_error("This world went totally insane. =("));
				}
			}
		};

#define DETAIL_TOOLKIT_DECL_BSRP_OPERATOR(Op_) \
			template < typename T, typename ByteDataType_, bool BigEndian, int Size > T operator Op_ (const BasicBitStreamReadProxy<ByteDataType_, BigEndian, Size>& bsrp, T val) { return static_cast<T>(bsrp) Op_ val; } \
			template < typename T, typename ByteDataType_, bool BigEndian, int Size > T operator Op_ (T val, const BasicBitStreamReadProxy<ByteDataType_, BigEndian, Size>& bsrp) { return val Op_ static_cast<T>(bsrp); }

		DETAIL_TOOLKIT_DECL_BSRP_OPERATOR(+)
		DETAIL_TOOLKIT_DECL_BSRP_OPERATOR(-)
		DETAIL_TOOLKIT_DECL_BSRP_OPERATOR(*)
		DETAIL_TOOLKIT_DECL_BSRP_OPERATOR(/)

#undef DETAIL_TOOLKIT_DECL_BSRP_OPERATOR
	}

	template < typename ByteDataType_, bool BigEndian = true >
	class BasicBitStream
	{
		typedef ByteDataType_ 	ByteDataType;

	public:
		template < int Size >
		class DeferredWriter
		{
		private:
			BasicBitStream*	_bitStream;
			size_t			_offset;

		public:
			DeferredWriter(BasicBitStream& bitStream)
				: _bitStream(&bitStream), _offset(bitStream._offset)
			{ _bitStream->_offset += Size; }

			template < typename T >
			void Write(T val)
			{ _bitStream->DoWrite<Size, T>(val, _offset); }
		};

	private:
		ByteDataType			_buf;
		size_t					_offset;

	public:
		BasicBitStream(const ByteDataType &buf) : _buf(buf), _offset(0) { }
		BasicBitStream(const ByteDataType &buf, size_t bytesOffset) : _buf(buf, bytesOffset), _offset(0) { }
		BasicBitStream(const ByteDataType &buf, size_t bytesOffset, size_t size) : _buf(buf, bytesOffset, size), _offset(0) { }
		BasicBitStream(const BasicBitStream &bitStream, size_t bytesOffset) : _buf(bitStream._buf, bytesOffset), _offset(0) { }
		BasicBitStream(const BasicBitStream &bitStream, size_t bytesOffset, size_t size) : _buf(bitStream._buf, bytesOffset, size), _offset(0) { }

		inline bool AtEnd() const						{ return (_offset >> 3) >= _buf.size(); }
		inline bool CanRead(size_t more_bits)			{ return (_offset + more_bits) <= (_buf.size() << 3); }
		inline size_t GetRemainingBitsCount() const		{ return (_buf.size() << 3) - _offset; }

		template<int Size>
		Detail::BasicBitStreamReadProxy<ByteDataType_, BigEndian, Size> Read()
		{
			size_t offset = _offset;
			_offset += Size;
			return Detail::BasicBitStreamReadProxy<ByteDataType_, BigEndian, Size>(&_buf, offset);
		}

		template < int Size >
		Detail::BasicBitStreamReadProxy<ByteDataType_, BigEndian, Size> Peek()
		{ return Detail::BasicBitStreamReadProxy<ByteDataType_, BigEndian, Size>(&_buf, _offset); }

		std::string ReadString(size_t length)
		{
			std::string result;
			for (size_t i = 0; i < length; ++i)
				result.push_back(Read<8>());
			return result;
		}

		std::string ReadStringTerminatedBy(char terminator)
		{
			std::string result;
			char c = Read<8>();
			while (c != terminator)
			{
				result.push_back(c);
				c = Read<8>();
			}
			return result;
		}

		std::string ReadZString()
		{ return ReadStringTerminatedBy('\0'); }

		template < int ElementSize, typename OutputIterator >
		void ReadArray(OutputIterator it, size_t count)
		{
			typedef typename Detail::GetIteratorValueType<OutputIterator>::ValueT	ValueType;
			for (size_t i = 0; i < count; ++i)
				*it++ = (ValueType)Read<ElementSize>();
		}

		template < int ElementSize, typename InputIterator >
		void WriteArray(InputIterator it, size_t count)
		{
			typedef typename Detail::GetIteratorValueType<InputIterator>::ValueT	ValueType;
			for (size_t i = 0; i < count; ++i)
				Write<ElementSize>(*it++);
		}

		template < int ElementSize, typename InputIterator >
		void WriteArray(InputIterator begin, InputIterator end)
		{
			typedef typename Detail::GetIteratorValueType<InputIterator>::ValueT	ValueType;
			for (; begin != end; ++begin)
				Write<ElementSize>(*begin);
		}

		template < int Size, typename T >
		void Write(T value)
		{
			DoWrite<Size, T>(value, _offset);
			_offset += Size;
		}

		inline void Seek(size_t bits) { _offset = bits; }
		inline void Skip(size_t bits) { _offset += bits; }

		inline size_t GetBitPosition() const { return _offset; }
		inline ByteDataType	GetData() const { return _buf; }

	private:
		template < int Size, typename T >
		void DoWrite(T value, size_t offset)
		{
			BasicBitsSetter<ByteDataType, BigEndian> bits(_buf, offset >> 3);
			switch(offset & 7)
			{
				case 0:		bits.template Set<0, Size>(value); break;
				case 1:		bits.template Set<1, Size>(value); break;
				case 2:		bits.template Set<2, Size>(value); break;
				case 3:		bits.template Set<3, Size>(value); break;
				case 4:		bits.template Set<4, Size>(value); break;
				case 5:		bits.template Set<5, Size>(value); break;
				case 6:		bits.template Set<6, Size>(value); break;
				case 7:		bits.template Set<7, Size>(value); break;
			}
		}
	};


	typedef BasicBitStream<Detail::UnsafeConstByteData>			UnsafeConstBitStream;
	typedef BasicBitStream<Detail::UnsafeByteData>				UnsafeBitStream;
	typedef BasicBitStream<Detail::UnsafeConstByteData, false>	UnsafeConstLittleEndianBitStream;
	typedef BasicBitStream<Detail::UnsafeByteData, false>		UnsafeLittleEndianBitStream;

	typedef BasicBitStream<ConstByteData>			ConstBitStream;
	typedef BasicBitStream<ByteData>				BitStream;
	typedef BasicBitStream<ConstByteData, false>	ConstLittleEndianBitStream;
	typedef BasicBitStream<ByteData, false>			LittleEndianBitStream;

	typedef BasicBitStream<ConstByteArray>			ConstSeisedBitStream;
	typedef BasicBitStream<ByteArray>				SeisedBitStream;
	typedef BasicBitStream<ConstByteArray, false>	ConstSeisedLittleEndianBitStream;
	typedef BasicBitStream<ByteArray, false>		SeisedLittleEndianBitStream;

}

#endif
