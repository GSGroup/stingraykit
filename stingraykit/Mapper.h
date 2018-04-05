#ifndef STINGRAYKIT_MAPPER_H
#define STINGRAYKIT_MAPPER_H

// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/string/ToString.h>

namespace stingray
{


	namespace Detail
	{
		template<typename Src, typename Dst>
		struct MappingBase
		{
			static bool HasValue(Src from)	{ return true; }
		};

		template<typename Src, typename Dst, bool IsSrcStringRepresentable = IsStringRepresentable<Src>::Value>
		struct FailImpl
		{
			static Dst GetValue(Src from)
			{ STINGRAYKIT_THROW(StringBuilder() % "Mapping " % Demangle(typeid(Src).name()) % " -> " % Demangle(typeid(Dst).name()) % " failed! Source value: " % ToString(from)); }
			static bool HasValue(Src from)	{ return false; }
		};

		template<typename Src, typename Dst>
		struct FailImpl<Src, Dst, false>
		{
			static Dst GetValue(Src from)
			{ STINGRAYKIT_THROW(StringBuilder() % "Mapping " % Demangle(typeid(Src).name()) % " -> " % Demangle(typeid(Dst).name()) % " failed! Source value is not string representable!"); }
			static bool HasValue(Src from)	{ return false; }
		};

		template<typename Dst>
		struct NoValue
		{
			static bool Fits(Dst val)	{ return false; }
		};

		template<typename Src, typename Dst>
		struct Mappings
		{
			template<Dst Start_, Dst End_, Dst MapBackValue_ = Start_>
			struct Range : public MappingBase<Src, Dst>
			{
				static bool Fits(Dst val)
				{
					CompileTimeAssert<(Start_ < End_)> ErrorStartAfterEnd;
					return Start_ <= val && val < End_;
				}
				static Dst GetValue(Src from)	{ return MapBackValue_; }
			};

			template<Dst Preferred, Dst Other>
			struct BiValue : public MappingBase<Src, Dst>
			{
				static bool Fits(Dst val)
				{
					CompileTimeAssert<Preferred != Other> ERROR_same_values;
					return val == Preferred || val == Other;
				}
				static Dst GetValue(Src from) { return Preferred; }
			};

			template<Dst Preferred, Dst Other1, Dst Other2>
			struct TriValue : public MappingBase<Src, Dst>
			{
				static bool Fits(Dst val)		{ return val == Preferred || val == Other1 || val == Other2; }
				static Dst GetValue(Src from)	{ return Preferred; }
			};

			template<Dst Preferred, Dst Other1, Dst Other2, Dst Other3>
			struct QuadValue : public MappingBase<Src, Dst>
			{
				static bool Fits(Dst val)		{ return val == Preferred || val == Other1 || val == Other2 || val == Other3; }
				static Dst GetValue(Src from)	{ return Preferred; }
			};

			template<Dst Preferred, typename Next = NoValue<Dst> >
			struct MultiValue : public MappingBase<Src, Dst>
			{
				static bool Fits(Dst val)		{ return val == Preferred || Next::Fits(val); }
				static Dst GetValue(Src from)	{ return Preferred; }
			};

			template<Dst Value_>
			struct Value : public MappingBase<Src, Dst>
			{
				static bool Fits(Dst val)		{ return val == Value_; }
				static Dst GetValue(Src from)	{ return Value_; }
			};

			typedef FailImpl<Src, Dst> Fail;
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
					to = MappingTo::GetValue(from);
				return !result; // need to negate since ForIf continues untill gets "false" and we need to stop when found fitting mapping
			}

			template<int N, bool IsMapBack>
			static bool DetailDoHasMapping(typename If<IsMapBack, DstT, SrcT>::ValueT from, bool& res)
			{
				typedef typename GetTypeListItem<List, N * 2 + (IsMapBack ? 1 : 0)>::ValueT	MappingFrom;
				typedef typename GetTypeListItem<List, N * 2 + (IsMapBack ? 0 : 1)>::ValueT	MappingTo;
				bool result = MappingFrom::Fits(from);
				if (result)
					res = MappingTo::HasValue(from);
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

				DstT result = static_cast<DstT>(0);
				if (ForIf<GetTypeListLength<List>::Value / 2, MapFunctor>::Do(val, ref(result)))
					return DefaultValue::GetValue(val);
				return result;
			}

			static SrcT Unmap(DstT val)
			{
				DetailCheckSizes();
				typedef typename GetTypeListItem<Default, 0>::ValueT DefaultValue;

				SrcT result = static_cast<SrcT>(0);
				if (ForIf<GetTypeListLength<List>::Value / 2, UnmapFunctor>::Do(val, ref(result)))
					return DefaultValue::GetValue(val);
				return result;
			}

			static bool HasMapping(SrcT val)
			{
				DetailCheckSizes();
				typedef typename GetTypeListItem<Default, 1>::ValueT DefaultValue;

				bool result = false;
				if (ForIf<GetTypeListLength<List>::Value / 2, HasMappingFunctor>::Do(val, ref(result)))
					result = DefaultValue::HasValue(val);
				return result;
			}

			static bool HasBackMapping(DstT val)
			{
				DetailCheckSizes();
				typedef typename GetTypeListItem<Default, 0>::ValueT DefaultValue;

				bool result = false;
				if (ForIf<GetTypeListLength<List>::Value / 2, HasBackMappingFunctor>::Do(val, ref(result)))
					result = DefaultValue::HasValue(val);
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
		typedef stingray::Detail::Mappings<DstType, SrcType> Src; // Short names are used for sake of mapping list readability: TypeList<Src::Value<123>, Dst::Range<1, 2, 3> >
		typedef stingray::Detail::Mappings<SrcType, DstType> Dst;

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
			CompileTimeAssert<IsInherited<Derived, ThisT>::Value> ERR_DerivedDoesntInherit_BaseValueMapper;
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
