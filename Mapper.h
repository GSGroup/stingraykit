#ifndef __GS_DVRLIB_TOOLKIT_MAPPER_H__
#define	__GS_DVRLIB_TOOLKIT_MAPPER_H__

namespace stingray {


namespace Detail
{
	template<typename T>
	struct Mappings
	{
		template<T Start_, T Center_, T End_>
		struct Range
		{
			static bool Fits(T val)
			{
				CompileTimeAssert<Start_ < End_> ERROR_start_after_end;
				return Start_ <= val && val < End_;
			}
			static T GetValue() { return Center_; }
		};

		template<T Preferred, T Other>
		struct MultiValue
		{
			static bool Fits(T val)
			{
				CompileTimeAssert<Preferred != Other> ERROR_same_values;
				return val == Preferred || val == Other;
			}
			static T GetValue() { return Preferred; }
		};


		template<T Value_>
		struct Value
		{
			static bool Fits(T val)	{ return val == Value_; }
			static T GetValue()		{ return Value_; }
		};

		struct Fail
		{
			static T GetValue()		{ TOOLKIT_THROW(std::runtime_error("Mapping failed!")); }
		};
	};

	template<typename SrcT, typename DstT, typename List, typename Default>
	struct Mapper
	{
		template<int N, bool IsMapBack>
		static bool DetailDoMap(typename If<IsMapBack, DstT, SrcT>::ValueT from, typename If<IsMapBack, SrcT, DstT>::ValueT& to)
		{
			typedef typename GetTypeListItem<List, N * 2 + (IsMapBack ? 1 : 0)>::ValueT	MappingFrom;
			typedef typename GetTypeListItem<List, N * 2 + (IsMapBack ? 0 : 1)>::ValueT	MappingTo;
			bool result = MappingFrom::Fits(from);
			if (result)
				to = MappingTo::GetValue();
			return !result; // need to negate since ForIf continues untill gets "false" and we need to stop when found fitting mapping
		}

		template<int N>
		struct MapFunctor
		{
			static bool Call(SrcT from, DstT& to) { return DetailDoMap<N, false>(from, to); }
		};

		template<int N>
		struct UnmapFunctor
		{
			static bool Call(DstT from, SrcT& to) { return DetailDoMap<N, true>(from, to); }
		};

		static DstT Map(SrcT val)
		{
			DetailCheckSizes();
			typedef typename GetTypeListItem<Default, 1>::ValueT DefaultValue;

			DstT result;
			if (ForIf<GetTypeListLength<List>::Value / 2, MapFunctor>::Do(val, ref(result)))
				return DefaultValue::GetValue();
			return result;
		}
		static SrcT Unmap(DstT val)
		{
			DetailCheckSizes();
			typedef typename GetTypeListItem<Default, 0>::ValueT DefaultValue;

			SrcT result;
			if (ForIf<GetTypeListLength<List>::Value / 2, UnmapFunctor>::Do(val, ref(result)))
				return DefaultValue::GetValue();
			return result;
		}
		static void DetailCheckSizes()
		{
			CompileTimeAssert<GetTypeListLength<List>::Value % 2 == 0> ERROR_wrong_mappings_count;
			CompileTimeAssert<GetTypeListLength<Default>::Value == 2> ERROR_wrong_default_mapping;
		}
	};
}

template <typename Derived, typename SrcT, typename DstT>
struct BaseValueMapper
{
	typedef SrcT SrcType;
	typedef DstT DstType;
	typedef stingray::Detail::Mappings<SrcType> Src; // Short names are used for sake of mapping list readability: TypeList_2<Src::Value<123>, Dst::Range<1, 2, 3> >
	typedef stingray::Detail::Mappings<DstType> Dst;

	static DstType Map(SrcType val)
	{
		typedef typename Derived::MappingsList MappingsList;
		typedef typename Derived::DefaultMapping DefaultMapping;

		return stingray::Detail::Mapper<SrcType, DstType, MappingsList, DefaultMapping>::Map(val);
	}
	static SrcType Unmap(DstType val)
	{
		typedef typename Derived::MappingsList MappingsList;
		typedef typename Derived::DefaultMapping DefaultMapping;

		return stingray::Detail::Mapper<SrcType, DstType, MappingsList, DefaultMapping>::Unmap(val);
	}
};


}

#endif
