#ifndef STINGRAYKIT_IO_BITSTREAM_H
#define STINGRAYKIT_IO_BITSTREAM_H

// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <iterator>

#include <stingraykit/Types.h>
#include <stingraykit/BitsGetter.h>

namespace stingray
{

	/**
	 * @ingroup toolkit_bits
	 * @defgroup toolkit_bits_bitstream Bitstreams
	 * @{
	 */

	namespace Detail
	{
		STINGRAYKIT_DECLARE_NESTED_TYPE_CHECK(container_type);

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
				default:	STINGRAYKIT_THROW(std::runtime_error("This world went totally insane. =("));
				}
			}
		};

#define DETAIL_STINGRAYKIT_DECL_BSRP_OPERATOR(Op_) \
			template < typename T, typename ByteDataType_, bool BigEndian, int Size > T operator Op_ (const BasicBitStreamReadProxy<ByteDataType_, BigEndian, Size>& bsrp, T val) { return static_cast<T>(bsrp) Op_ val; } \
			template < typename T, typename ByteDataType_, bool BigEndian, int Size > T operator Op_ (T val, const BasicBitStreamReadProxy<ByteDataType_, BigEndian, Size>& bsrp) { return val Op_ static_cast<T>(bsrp); }

		DETAIL_STINGRAYKIT_DECL_BSRP_OPERATOR(+)
		DETAIL_STINGRAYKIT_DECL_BSRP_OPERATOR(-)
		DETAIL_STINGRAYKIT_DECL_BSRP_OPERATOR(*)
		DETAIL_STINGRAYKIT_DECL_BSRP_OPERATOR(/)
		DETAIL_STINGRAYKIT_DECL_BSRP_OPERATOR(>>)
		DETAIL_STINGRAYKIT_DECL_BSRP_OPERATOR(<<)

#undef DETAIL_STINGRAYKIT_DECL_BSRP_OPERATOR
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

		class Watcher
		{
		private:
			BasicBitStream*	_stream;
			size_t			_offset;

		public:
			Watcher(BasicBitStream& stream)
				: _stream(&stream), _offset(stream._offset)
			{ }

			size_t GetOffsetDelta() const
			{ return _stream->_offset - _offset; }

			ByteDataType GetDataDelta() const
			{ return ByteDataType(_stream->_buf, _offset / 8, (_stream->_offset - _offset) / 8); }
		};

		class Rollback
		{
		private:
			BasicBitStream*	_stream;
			size_t			_offset;

		public:
			Rollback(BasicBitStream& stream)
				: _stream(&stream), _offset(stream._offset)
			{ }

			~Rollback()
			{ _stream->Seek(_offset); }
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

		template < size_t LengthPrefixSize >
		std::string ReadLengthPrefixedString()
		{
			std::string result;

			const size_t length = Read<LengthPrefixSize>();
			result.reserve(length);
			for (size_t i = 0; i < length; ++i)
				result.push_back(Read<8>());

			return result;
		}

		template < size_t LengthPrefixSize >
		void WriteLengthPrefixedString(const std::string& str)
		{
			Write<LengthPrefixSize>(str.length());
			WriteArray<8>(str.begin(), str.end());
		}

		template < size_t LengthPrefixSize >
		ByteArray ReadLengthPrefixedArray()
		{
			size_t size = Read<LengthPrefixSize>();
			ByteArray result(size);
			ReadArray<8>(result.begin(), result.end());
			return result;
		}

		template < size_t LengthPrefixSize >
		void WriteLengthPrefixedArray(const ConstByteArray &data)
		{
			Write<LengthPrefixSize>(data.size());
			WriteArray<8>(data.begin(), data.end());
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

		void WriteNullTerminatedString(const std::string& str)
		{
			WriteArray<8>(str.begin(), str.end());
			Write<8>('\0');
		}

		std::string ReadNullTerminatedString()
		{ return ReadStringTerminatedBy('\0'); }

		template < int ElementSize, typename OutputIterator >
		void ReadArray(OutputIterator it, size_t count)
		{
			typedef typename Detail::GetIteratorValueType<OutputIterator>::ValueT	ValueType;
			for (size_t i = 0; i < count; ++i)
				*it++ = (ValueType)Read<ElementSize>();
		}

		template < int ElementSize, typename OutputIterator >
		void ReadArray(OutputIterator first, OutputIterator last)
		{
			for ( ; first != last; first++)
				*first = Read<ElementSize>();
		}

		template < int ElementSize, typename InputIterator >
		void WriteArray(InputIterator it, size_t count)
		{
			for (size_t i = 0; i < count; ++i)
				Write<ElementSize>(*it++);
		}

		template < int ElementSize, typename InputIterator >
		void WriteArray(InputIterator first, InputIterator last)
		{
			for ( ; first != last; first++)
				Write<ElementSize>(*first);
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
		inline ByteDataType GetAvailableByteData() const
		{
			STINGRAYKIT_CHECK((GetBitPosition() % 8) == 0, "misaligned stream");
			return ByteDataType(GetData(), GetBitPosition() / 8);
		}

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

	/** @} */

}

#endif
