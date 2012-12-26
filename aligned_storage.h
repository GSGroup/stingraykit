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
			struct a2	{ u16 s; };
			struct a4	{ u32 s; };
			struct a8	{ u64 s; };
			struct a16	{ long double s; };
		}

		template <std::size_t N> struct type_with_alignment;
		template<> class type_with_alignment<1>  { public: typedef char type; };
		template<> class type_with_alignment<2>  { public: typedef AlignedTypes::a2 type; };
		template<> class type_with_alignment<4>  { public: typedef AlignedTypes::a4 type; };
		template<> class type_with_alignment<8>  { public: typedef AlignedTypes::a8 type; };
		template<> class type_with_alignment<16> { public: typedef AlignedTypes::a16 type; };

		template<size_t Len, size_t Align>
		struct AlignedStorageImpl
		{
			union data_t
			{
				char						_buf[Len];
				type_with_alignment<Align>	_align;
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


}


#endif

