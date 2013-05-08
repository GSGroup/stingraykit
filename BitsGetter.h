#ifndef STINGRAY_TOOLKIT_BITSGETTER_H
#define STINGRAY_TOOLKIT_BITSGETTER_H

#include <limits>

#include <stingray/toolkit/ByteData.h>
#include <stingray/toolkit/toolkit.h>
#include <stingray/toolkit/exception.h>
#include <stingray/toolkit/Dummy.h>


/*! \cond GS_INTERNAL */

namespace stingray
{

	namespace Detail
	{
		template < typename T >
		struct ShiftableType { typedef typename IntType<sizeof(T) * 8, true>::ValueT ValueT; };

		template < >
		struct ShiftableType<bool> { typedef u32 ValueT; };

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
			static FORCE_INLINE T mask(unsigned len) {
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
				TOOLKIT_CHECK(_buf.size() >= BytesRequirement, IndexOutOfRangeException(BytesRequirement, _buf.size()));

				typedef typename ShiftableType<T>::ValueT	ShiftableT;

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
			FORCE_INLINE operator T() const
			{
				CompileTimeAssert<!SameType<T, BitsGetterProxy>::Value >	ERROR__old_gcc_bug;
				(void)ERROR__old_gcc_bug;

				// In case of negative SizeBits or OffsetBits index check can pass due to overflow
				CompileTimeAssert<SizeBits + OffsetBits >= SizeBits>();
				CompileTimeAssert<SizeBits + OffsetBits >= OffsetBits>();

				const size_t BytesRequirement = (OffsetBits + SizeBits + 7) / 8;
				TOOLKIT_CHECK(_buf.size() >= BytesRequirement, IndexOutOfRangeException(BytesRequirement, _buf.size()));

				typedef typename ShiftableType<T>::ValueT	ShiftableT;

				ShiftableT result = ShiftableT();

				const size_t offset = OffsetBits / 8;
				const size_t count = SizeBits / 8;
				for(size_t i = 0; i < count; ++i)
					result = (result << 8) | _buf[offset + (BigEndian? i : count - i - 1)];

				return static_cast<typename BitsGetterResultType<T>::ValueT>(result);
			}

		};

#define DETAIL_TOOLKIT_DECL_BGP_OPERATOR(Op_) \
		template < typename T, typename ByteDataType_, bool BigEndian, size_t OffsetBits, size_t SizeBits, bool UseMasks > T operator Op_ (BitsGetterProxy<ByteDataType_, BigEndian, OffsetBits, SizeBits, UseMasks> bgp, T val) { return static_cast<T>(bgp) Op_ val; } \
		template < typename T, typename ByteDataType_, bool BigEndian, size_t OffsetBits, size_t SizeBits, bool UseMasks > T operator Op_ (T val, BitsGetterProxy<ByteDataType_, BigEndian, OffsetBits, SizeBits, UseMasks> bgp) { return val Op_ static_cast<T>(bgp); }

	DETAIL_TOOLKIT_DECL_BGP_OPERATOR(+)
	DETAIL_TOOLKIT_DECL_BGP_OPERATOR(-)
	DETAIL_TOOLKIT_DECL_BGP_OPERATOR(*)
	DETAIL_TOOLKIT_DECL_BGP_OPERATOR(/)
	DETAIL_TOOLKIT_DECL_BGP_OPERATOR(<<)
	DETAIL_TOOLKIT_DECL_BGP_OPERATOR(>>)
	DETAIL_TOOLKIT_DECL_BGP_OPERATOR(==)
	DETAIL_TOOLKIT_DECL_BGP_OPERATOR(!=)

#undef DETAIL_TOOLKIT_DECL_BGP_OPERATOR

		TOOLKIT_DECLARE_METHOD_CHECK(RequireSize);

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
			static FORCE_INLINE T mask(unsigned len) {
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
				TOOLKIT_CHECK(required_size <= _buf.size(), IndexOutOfRangeException(required_size, _buf.size()));

				typedef typename ShiftableType<T>::ValueT	ShiftableT;
				typedef typename BitsGetterResultType<T>::ValueT	ResType;

				ShiftableT shiftable_val = static_cast<ShiftableT>(static_cast<ResType>(val));

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
			FORCE_INLINE void Set (T val) const
			{
				// In case of negative SizeBits or OffsetBits index check can pass due to overflow
				CompileTimeAssert<SizeBits + OffsetBits >= SizeBits>();
				CompileTimeAssert<SizeBits + OffsetBits >= OffsetBits>();

				size_t required_size = (OffsetBits + SizeBits + 7) / 8;
				ByteDataResizer<ByteDataType>::RequireSize(_buf, required_size);
				TOOLKIT_CHECK(required_size <= _buf.size(), IndexOutOfRangeException(required_size, _buf.size()));

				typedef typename ShiftableType<T>::ValueT			ShiftableT;
				typedef typename BitsGetterResultType<T>::ValueT	ResType;

				ShiftableT shiftable = static_cast<ShiftableT>(static_cast<ResType>(val));

				const size_t offset = OffsetBits / 8;
				const size_t count = SizeBits / 8;
				for(int i = count - 1; i >= 0; --i)
				{
					_buf[offset + (BigEndian? i : count - i - 1)] = static_cast<u8>(shiftable);
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

		template < size_t OffsetBits, size_t SizeBits, typename T >
		void Set(T val) const
		{ Detail::BitsSetterImpl<ByteDataType, BigEndian, OffsetBits, SizeBits>(_buf).Set(val); }

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

}

/*! \endcond */


#endif
