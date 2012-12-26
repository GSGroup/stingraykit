#ifndef STINGRAY_TOOLKIT_ALIGNED_STORAGE_H
#define STINGRAY_TOOLKIT_ALIGNED_STORAGE_H

#include <stingray/toolkit/Types.h>

namespace stingray
{

	namespace Detail
	{
		namespace EnlargementAlignmentMeasurer
		{
			template <typename T> struct alignment_of;

			template <typename T, int size_diff>
			struct AlignmentOfHelper
			{ static const size_t value = size_diff; };

			template <typename T>
			struct AlignmentOfHelper<T, 0>
			{ static const size_t value = alignment_of<T>::Value; };

			template <typename T>
			struct alignment_of
			{
				struct Enlarged { T x; char c; };

				static const size_t diff = sizeof(Enlarged) - sizeof(T);
				static const size_t value = AlignmentOfHelper<Enlarged, diff>::value;
			};
		}

		namespace SimpleAlignmentMeasurer
		{
			template <typename T>
			struct alignment_of
			{
				static const size_t size = sizeof(T);
				static const size_t value = size ^ (size & (size - 1));
			};
		}

		namespace AlignedTypes
		{
			struct a1	{ u8 s; };
			struct a2	{ u16 s; };
			struct a4	{ u32 s; };
			struct a8	{ u64 s; };
			struct a16	{ long double s; };
		}

		template <std::size_t N> struct type_with_alignment;
		template<> struct type_with_alignment<1>  { typedef AlignedTypes::a1 type; };
		template<> struct type_with_alignment<2>  { typedef AlignedTypes::a2 type; };
		template<> struct type_with_alignment<4>  { typedef AlignedTypes::a4 type; };
		template<> struct type_with_alignment<8>  { typedef AlignedTypes::a8 type; };
		template<> struct type_with_alignment<16> { typedef AlignedTypes::a16 type; };

		template<size_t Len, size_t Align>
		struct AlignedStorageImpl
		{
			union data_t
			{
				char										_buf[Len];
				typename type_with_alignment<Align>::type	_align;
			} _data;
		};
	}


    template <typename T>
    struct alignment_of
    {
       static const size_t simple = Detail::SimpleAlignmentMeasurer::alignment_of<T>::value;
	   static const size_t enlarged = Detail::EnlargementAlignmentMeasurer::alignment_of<T>::value;
	   static const size_t value = simple < enlarged ? simple : enlarged;
    };


	template<size_t Len, size_t Align>
	struct aligned_storage
	{
		typedef Detail::AlignedStorageImpl<Len, Align> type;
	};


	template<typename T>
	struct StorageFor
	{
		typename aligned_storage<sizeof(T), alignment_of<T>::value>::type _value;

		void Ctor() { new(&Ref()) T(); }

		template < typename P1 >
		void Ctor(const P1& p1) { new(&Ref()) T(p1); }

		template < typename P1, typename P2 >
		void Ctor(const P1& p1, const P2& p2) { new(&Ref()) T(p1, p2); }

		template < typename P1, typename P2, typename P3 >
		void Ctor(const P1& p1, const P2& p2, const P3& p3) { new(&Ref()) T(p1, p2, p3); }

		void Dtor()
		{ Ref().~T(); }

		T& Ref()				{ return reinterpret_cast<T&>(_value); }
		const T& Ref() const	{ return reinterpret_cast<const T&>(_value); }
	};



}


#endif

