#ifndef STINGRAYKIT_BITSGETTER_H
#define STINGRAYKIT_BITSGETTER_H

// Copyright (c) 2011 - 2015, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <limits>
#if defined(__GNUC__) || defined(__clang__)
#	include <byteswap.h>
#endif

#include <stingraykit/collection/ByteData.h>
#include <stingraykit/toolkit.h>
#include <stingraykit/exception.h>
#include <stingraykit/Dummy.h>


namespace stingray
{

	/**
	 * @ingroup toolkit_bits
	 * @defgroup toolkit_bits_bitsgetter BitsGetter
	 * @{
	 */

	namespace Detail
	{
		template < typename T >
		struct ShiftableType { typedef typename IntType<sizeof(T) * 8, false>::ValueT ValueT; };

		template < >
		struct ShiftableType<bool> { typedef u32 ValueT; };

		template<size_t SizeBits>
		struct MinimalTypeSelector
		{
			static const size_t SizeBytes								= (SizeBits + 7) / 8;
			static const size_t AlignedSize								= (SizeBytes > 4? 8: (SizeBytes > 2? 4: (SizeBytes > 1? 2: 1)));
			typedef typename IntType<AlignedSize * 8, false>::ValueT	ValueT;
		};

#if defined(__GNUC__) || defined(__clang__)
		template < typename T >
		struct SwappableType;

		template < > struct SwappableType<u8>
		{ static u8 Swap(u8 value) { return value; } };

		template < > struct SwappableType<u16>
		{ static u16 Swap(u16 value) { return __bswap_16(value); } };

		template < > struct SwappableType<u32>
		{ static u32 Swap(u32 value) { return __bswap_32(value); } };

		template < > struct SwappableType<u64>
		{ static u64 Swap(u64 value) { return __bswap_64(value); } };
#else
		template < typename T >
		struct SwappableType
		{
			static T Swap(T value)
			{
				T r = 0;
				for(size_t i = 0; i < sizeof(T); ++i)
				{
					T byte = (value >> (8 * i)) & 0xff;
					r |= (byte << ((sizeof(T) - i - 1) * 8));
				}
				return r;
			}
		};

#endif

		template < typename T, bool TIsEnumClass = IsEnumClass<T>::Value >
		struct BitsGetterResultType
		{ typedef T ValueT; };

		template < typename T >
		struct BitsGetterResultType<T, true>
		{ typedef typename T::Enum ValueT; };


		template < typename T >
		class BasicUnsafeByteData
		{
		private:
			T*		_data;

		public:
			BasicUnsafeByteData(T* data, size_t ofs = 0) : _data(data + ofs) { }
			BasicUnsafeByteData(const BasicUnsafeByteData& data, size_t ofs) : _data(data._data + ofs) { }

			T& operator [] (size_t i) const	{ return _data[i]; }
			size_t size() const				{ return std::numeric_limits<size_t>::max(); }
		};

		typedef BasicUnsafeByteData<u8>			UnsafeByteData;
		typedef BasicUnsafeByteData<const u8>	UnsafeConstByteData;


		template < typename ByteDataType_, bool BigEndian, size_t OffsetBits, size_t SizeBits, bool UseMasks = ((OffsetBits | SizeBits) & 7) != 0>
		class BitsGetterProxy
		{
			typedef ByteDataType_	ByteDataType;

		private:
			ByteDataType _buf;

			template<typename T>
			static inline T mask(unsigned len) {
				return len >= (8 * sizeof(T))? ~T(): ((T(1) << len) - 1);
			}

		public:
			BitsGetterProxy(const ByteDataType &buf) : _buf(buf) { }

			template < typename T >
			inline operator T() const
			{
				CompileTimeAssert<!SameType<T, BitsGetterProxy>::Value >	ERROR__old_gcc_bug;
				(void)ERROR__old_gcc_bug;
				CompileTimeAssert<SizeBits <= 64>();

				// In case of negative SizeBits or OffsetBits index check can pass due to overflow
				CompileTimeAssert<SizeBits + OffsetBits >= SizeBits>();
				CompileTimeAssert<SizeBits + OffsetBits >= OffsetBits>();

				const size_t BytesRequirement = (OffsetBits + SizeBits + 7) / 8;
				STINGRAYKIT_CHECK(_buf.size() >= BytesRequirement, IndexOutOfRangeException(BytesRequirement, _buf.size()));

				typedef typename ShiftableType<T>::ValueT					ShiftableT;
				typedef typename MinimalTypeSelector<SizeBits>::ValueT		MinimalT;

				ShiftableT result = ShiftableT();
				size_t cur_byte = OffsetBits / 8;
				int msb = 8 - (OffsetBits % 8);

				int count = SizeBits;
				while (count > 0)
				{
					int shift = msb - count;
					if(shift >= 0)
					{
						result |= (ShiftableT(_buf[cur_byte]) >> shift) & mask<ShiftableT>(count);
						break;
					}
					else
					{
						result |= (ShiftableT(_buf[cur_byte]) & mask<ShiftableT>(msb)) << -shift;
						count -= msb;
						++cur_byte;
						msb = 8;
					}
				}
				if (!BigEndian)
					result = SwappableType<MinimalT>::Swap(result << (sizeof(MinimalT) * 8 - SizeBits)); //24 bit values are shifted left, so 11 22 33 becomes 00 33 22 11, not 33 22 11 00
				return static_cast<typename BitsGetterResultType<T>::ValueT>(result);
			}
		};


		template < typename ByteDataType_, bool BigEndian, size_t OffsetBits, size_t SizeBits >
		class BitsGetterProxy<ByteDataType_, BigEndian, OffsetBits, SizeBits, false>
		{
			typedef ByteDataType_	ByteDataType;

		private:
			ByteDataType	_buf;

		public:
			BitsGetterProxy(const ByteDataType &buf) : _buf(buf) { }

			template < typename T >
			inline operator T() const
			{
				CompileTimeAssert<!SameType<T, BitsGetterProxy>::Value >	ERROR__old_gcc_bug;
				(void)ERROR__old_gcc_bug;

				// In case of negative SizeBits or OffsetBits index check can pass due to overflow
				CompileTimeAssert<SizeBits + OffsetBits >= SizeBits>();
				CompileTimeAssert<SizeBits + OffsetBits >= OffsetBits>();

				const size_t BytesRequirement = (OffsetBits + SizeBits + 7) / 8;
				STINGRAYKIT_CHECK(_buf.size() >= BytesRequirement, IndexOutOfRangeException(BytesRequirement, _buf.size()));

				typedef typename ShiftableType<T>::ValueT					ShiftableT;
				typedef typename MinimalTypeSelector<SizeBits>::ValueT		MinimalT;

				ShiftableT result = ShiftableT();

				const size_t offset = OffsetBits / 8;
				const size_t count = SizeBits / 8;
				for(size_t i = 0; i < count; ++i)
					result = (result << 8) | _buf[offset + i];

				if (!BigEndian)
					result = SwappableType<MinimalT>::Swap(result << (sizeof(MinimalT) * 8 - SizeBits)); //24 bit values are shifted left, so 11 22 33 becomes 00 33 22 11, not 33 22 11 00
				return static_cast<typename BitsGetterResultType<T>::ValueT>(result);
			}

		};

#define DETAIL_STINGRAYKIT_DECL_BGP_OPERATOR(Op_) \
		template < typename T, typename ByteDataType_, bool BigEndian, size_t OffsetBits, size_t SizeBits, bool UseMasks > T operator Op_ (BitsGetterProxy<ByteDataType_, BigEndian, OffsetBits, SizeBits, UseMasks> bgp, T val) { return static_cast<T>(bgp) Op_ val; } \
		template < typename T, typename ByteDataType_, bool BigEndian, size_t OffsetBits, size_t SizeBits, bool UseMasks > T operator Op_ (T val, BitsGetterProxy<ByteDataType_, BigEndian, OffsetBits, SizeBits, UseMasks> bgp) { return val Op_ static_cast<T>(bgp); }

	DETAIL_STINGRAYKIT_DECL_BGP_OPERATOR(+)
	DETAIL_STINGRAYKIT_DECL_BGP_OPERATOR(-)
	DETAIL_STINGRAYKIT_DECL_BGP_OPERATOR(*)
	DETAIL_STINGRAYKIT_DECL_BGP_OPERATOR(/)
	DETAIL_STINGRAYKIT_DECL_BGP_OPERATOR(<<)
	DETAIL_STINGRAYKIT_DECL_BGP_OPERATOR(>>)
	DETAIL_STINGRAYKIT_DECL_BGP_OPERATOR(==)
	DETAIL_STINGRAYKIT_DECL_BGP_OPERATOR(!=)

#undef DETAIL_STINGRAYKIT_DECL_BGP_OPERATOR

		STINGRAYKIT_DECLARE_METHOD_CHECK(RequireSize);

		template < typename ByteDataType_, bool CanResize = HasMethod_RequireSize<ByteDataType_>::Value >
		struct ByteDataResizer
		{ static void RequireSize(ByteDataType_& data, size_t size) { data.RequireSize(size); } };

		template < typename ByteDataType_ >
		struct ByteDataResizer<ByteDataType_, false>
		{ static void RequireSize(ByteDataType_&, size_t) { } };

		template < typename ByteDataType_, bool BigEndian, size_t OffsetBits, size_t SizeBits, bool UseMasks = ((OffsetBits | SizeBits) & 7) != 0 >
		class BitsSetterImpl
		{
			typedef ByteDataType_	ByteDataType;

		private:
			mutable ByteDataType _buf;

			template<typename T>
			static inline T mask(unsigned len) {
				return len >= (8 * sizeof(T))? ~T(): ((T(1) << len) - 1);
			}

		public:
			BitsSetterImpl(const ByteDataType &buf) : _buf(buf) { }

			template < typename T >
			inline void Set (T val) const
			{
				// In case of negative SizeBits or OffsetBits index check can pass due to overflow
				CompileTimeAssert<SizeBits + OffsetBits >= SizeBits>();
				CompileTimeAssert<SizeBits + OffsetBits >= OffsetBits>();

				size_t required_size = (OffsetBits + SizeBits + 7) / 8;
				ByteDataResizer<ByteDataType>::RequireSize(_buf, required_size);
				STINGRAYKIT_CHECK(required_size <= _buf.size(), IndexOutOfRangeException(required_size, _buf.size()));

				typedef typename ShiftableType<T>::ValueT			ShiftableT;
				typedef typename BitsGetterResultType<T>::ValueT	ResType;
				typedef typename MinimalTypeSelector<SizeBits>::ValueT		MinimalT;

				ShiftableT shiftable_val = static_cast<ShiftableT>(static_cast<ResType>(val));
				if (!BigEndian)
					shiftable_val = SwappableType<MinimalT>::Swap(shiftable_val);

				size_t cur_byte = OffsetBits / 8;
				int msb = 8 - (OffsetBits % 8);

				int count = SizeBits;
				while (count > 0)
				{
					int shift = msb - count;
					if(shift >= 0)
					{
						//result |= (ShiftableT(_buf[cur_byte]) >> shift) & mask<ShiftableT>(count);
						_buf[cur_byte] &= ~(mask<ShiftableT>(count) << shift);
						_buf[cur_byte] |= (shiftable_val << shift) & (mask<ShiftableT>(count) << shift);
						break;
					}
					else
					{
						//result |= (ShiftableT(_buf[cur_byte]) & mask<ShiftableT>(msb)) << -shift;
						_buf[cur_byte] &= ~mask<ShiftableT>(msb);
						_buf[cur_byte] |= (shiftable_val >> -shift) & mask<ShiftableT>(msb);
						count -= msb;
						++cur_byte;
						msb = 8;
					}
				}
			}
		};

		template < typename ByteDataType_, bool BigEndian, size_t OffsetBits, size_t SizeBits >
		class BitsSetterImpl<ByteDataType_, BigEndian, OffsetBits, SizeBits, false>
		{
			typedef ByteDataType_	ByteDataType;

		private:
			mutable ByteDataType _buf;

		public:
			BitsSetterImpl(const ByteDataType &buf) : _buf(buf) { }

			template < typename T >
			inline void Set (T val) const
			{
				// In case of negative SizeBits or OffsetBits index check can pass due to overflow
				CompileTimeAssert<SizeBits + OffsetBits >= SizeBits>();
				CompileTimeAssert<SizeBits + OffsetBits >= OffsetBits>();

				size_t required_size = (OffsetBits + SizeBits + 7) / 8;
				ByteDataResizer<ByteDataType>::RequireSize(_buf, required_size);
				STINGRAYKIT_CHECK(required_size <= _buf.size(), IndexOutOfRangeException(required_size, _buf.size()));

				typedef typename ShiftableType<T>::ValueT			ShiftableT;
				typedef typename BitsGetterResultType<T>::ValueT	ResType;
				typedef typename MinimalTypeSelector<SizeBits>::ValueT		MinimalT;

				ShiftableT shiftable = static_cast<ShiftableT>(static_cast<ResType>(val));
				if (!BigEndian)
					shiftable = SwappableType<MinimalT>::Swap(shiftable);

				const size_t offset = OffsetBits / 8;
				const size_t count = SizeBits / 8;
				for(int i = count - 1; i >= 0; --i)
				{
					_buf[offset + i] = static_cast<u8>(shiftable);
					shiftable >>= 8;
				}
			}
		};

	}


	template < typename ByteDataType_, bool BigEndian = true >
	class BasicBitsGetter
	{
		typedef ByteDataType_		ByteDataType;

	private:
		ByteDataType	_buf;

	public:
		BasicBitsGetter(const ByteDataType &buf) : _buf(buf) { }
		BasicBitsGetter(const ByteDataType &buf, size_t bytesOffset) : _buf(buf, bytesOffset) { }
		BasicBitsGetter(const ByteDataType &buf, size_t bytesOffset, size_t size) : _buf(buf, bytesOffset, size) { }

		template < size_t OffsetBits, size_t SizeBits >
		Detail::BitsGetterProxy<ByteDataType, BigEndian,  OffsetBits, SizeBits> Get() const
		{ return Detail::BitsGetterProxy<ByteDataType, BigEndian,  OffsetBits, SizeBits>(_buf); }

		template < size_t OffsetBits, typename PodType >
		Detail::BitsGetterProxy<ByteDataType, BigEndian, OffsetBits, 8 * sizeof(PodType)> Get(const Dummy& dummy = Dummy()) const
		{ return Get<OffsetBits, 8 * sizeof(PodType)>(); }

		template < typename PodType >
		Detail::BitsGetterProxy<ByteDataType, BigEndian, 0, 8 * sizeof(PodType)> Get(const Dummy& dummy = Dummy(), const Dummy& dummy2 = Dummy()) const
		{ return Get<0, 8 * sizeof(PodType)>(); }
	};

	template < typename ByteDataType_ , bool BigEndian = true >
	class BasicBitsSetter
	{
		typedef ByteDataType_		ByteDataType;

	private:
		ByteDataType	_buf;

	public:
		BasicBitsSetter(const ByteDataType &buf) : _buf(buf) { }
		BasicBitsSetter(const ByteDataType &buf, size_t bytesOffset) : _buf(buf, bytesOffset) { }
		BasicBitsSetter(const ByteDataType &buf, size_t bytesOffset, size_t size) : _buf(buf, bytesOffset, size) { }
		BasicBitsSetter(ByteDataType &buf, size_t bytesOffset) : _buf(BufResizer(buf, bytesOffset), bytesOffset) { }
		BasicBitsSetter(ByteDataType &buf, size_t bytesOffset, size_t size) : _buf(BufResizer(buf, bytesOffset), bytesOffset, size) { }

		template < size_t OffsetBits, size_t SizeBits, typename PodType >
		void Set(PodType val) const
		{ Detail::BitsSetterImpl<ByteDataType, BigEndian, OffsetBits, SizeBits>(_buf).Set(val); }

		template < size_t OffsetBits, typename PodType >
		void Set(PodType val) const
		{ Set<OffsetBits, 8 * sizeof(PodType)>(val); }

		template < typename PodType >
		void Set(PodType val) const
		{ Set<0>(val); }

	private:
		static const ByteDataType& BufResizer(ByteDataType& buf, size_t minSize)
		{
			Detail::ByteDataResizer<ByteDataType>::RequireSize(buf, minSize);
			return buf;
		}
	};


	typedef BasicBitsGetter<Detail::UnsafeConstByteData>	UnsafeBitsGetter;
	typedef BasicBitsSetter<Detail::UnsafeByteData>			UnsafeBitsSetter;
	typedef BasicBitsGetter<ConstByteData>					BitsGetter;
	typedef BasicBitsSetter<ByteData>						BitsSetter;

	typedef BasicBitsGetter<Detail::UnsafeConstByteData, false>		UnsafeLittleEndianBitsGetter;
	typedef BasicBitsSetter<Detail::UnsafeByteData, false>			UnsafeLittleEndianBitsSetter;
	typedef BasicBitsGetter<ConstByteData, false>					LittleEndianBitsGetter;
	typedef BasicBitsSetter<ByteData, false>						LittleEndianBitsSetter;

	/** @} */
}


#endif
