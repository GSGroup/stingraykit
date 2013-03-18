#ifndef STINGRAY_TOOLKIT_MAPPER_H
#define STINGRAY_TOOLKIT_MAPPER_H

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
			struct BiValue
			{
				static bool Fits(T val)
				{
					CompileTimeAssert<Preferred != Other> ERROR_same_values;
					return val == Preferred || val == Other;
				}
				static T GetValue() { return Preferred; }
			};

			template<T Preferred, T Other1, T Other2>
			struct TriValue
			{
				static bool Fits(T val)	{ return val == Preferred || val == Other1 || val == Other2; }
				static T GetValue()		{ return Preferred; }
			};

			template<T Preferred, T Other1, T Other2, T Other3>
			struct QuadValue
			{
				static bool Fits(T val)	{ return val == Preferred || val == Other1 || val == Other2 || val == Other3; }
				static T GetValue()		{ return Preferred; }
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

			template<int N, bool IsMapBack>
			static bool DetailDoHasMapping(typename If<IsMapBack, DstT, SrcT>::ValueT from, bool& res)
			{
				typedef typename GetTypeListItem<List, N * 2 + (IsMapBack ? 1 : 0)>::ValueT	MappingFrom;
				typedef typename GetTypeListItem<List, N * 2 + (IsMapBack ? 0 : 1)>::ValueT	MappingTo;
				bool result = MappingFrom::Fits(from);
				if (result)
					res = !SameType<MappingTo, typename Mappings<typename If<IsMapBack, SrcT, DstT>::ValueT>::Fail>::Value;
				return !result; // need to negate since ForIf continues untill gets "false" and we need to stop when found fitting mapping
			}

			template<int N>
			struct MapFunctor
			{ static bool Call(SrcT from, DstT& to) { return DetailDoMap<N, false>(from, to); } };

			template<int N>
			struct UnmapFunctor
			{ static bool Call(DstT from, SrcT& to) { return DetailDoMap<N, true>(from, to); } };

			template<int N>
			struct HasMappingFunctor
			{ static bool Call(SrcT from, bool& res) { return DetailDoHasMapping<N, false>(from, res); } };

			template<int N>
			struct HasBackMappingFunctor
			{ static bool Call(DstT from, bool& res) { return DetailDoHasMapping<N, true>(from, res); } };

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

			static bool HasMapping(SrcT val)
			{
				DetailCheckSizes();
				typedef typename GetTypeListItem<Default, 1>::ValueT DefaultValue;

				bool result = false;
				if (ForIf<GetTypeListLength<List>::Value / 2, HasMappingFunctor>::Do(val, ref(result)))
					result = !SameType<Default, typename Mappings<SrcT>::Fail>::Value;
				return result;
			}

			static bool HasBackMapping(DstT val)
			{
				DetailCheckSizes();
				typedef typename GetTypeListItem<Default, 0>::ValueT DefaultValue;

				bool result = false;
				if (ForIf<GetTypeListLength<List>::Value / 2, HasBackMappingFunctor>::Do(val, ref(result)))
					result = !SameType<Default, typename Mappings<DstT>::Fail>::Value;
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
			Check();
			typedef typename Derived::MappingsList MappingsList;
			typedef typename Derived::DefaultMapping DefaultMapping;

			return stingray::Detail::Mapper<SrcType, DstType, MappingsList, DefaultMapping>::Map(val);
		}

		static SrcType Unmap(DstType val)
		{
			Check();
			typedef typename Derived::MappingsList MappingsList;
			typedef typename Derived::DefaultMapping DefaultMapping;

			return stingray::Detail::Mapper<SrcType, DstType, MappingsList, DefaultMapping>::Unmap(val);
		}

		static bool HasMapping(SrcType val)
		{
			Check();
			typedef typename Derived::MappingsList MappingsList;
			typedef typename Derived::DefaultMapping DefaultMapping;

			return stingray::Detail::Mapper<SrcType, DstType, MappingsList, DefaultMapping>::HasMapping(val);
		}

		static bool HasBackMapping(DstType val)
		{
			Check();
			typedef typename Derived::MappingsList MappingsList;
			typedef typename Derived::DefaultMapping DefaultMapping;

			return stingray::Detail::Mapper<SrcType, DstType, MappingsList, DefaultMapping>::HasBackMapping(val);
		}
	private:
		static void Check()
		{
			typedef BaseValueMapper<Derived, SrcT, DstT> ThisT;
			typedef CompileTimeAssert<Inherits<Derived, ThisT>::Value> ERR_DerivedDoesntInherit_BaseValueMapper;
		}
	};


	template <typename SrcT, typename DstT>
	struct IdentityMapper
	{
		typedef SrcT SrcType;
		typedef DstT DstType;
		static DstType Map(SrcType val)		{ return val; }
		static SrcType Unmap(DstType val)	{ return val; }
	};

}

#endif
