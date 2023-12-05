#ifndef STINGRAYKIT_COMPARE_COMPARERS_H
#define STINGRAYKIT_COMPARE_COMPARERS_H

// Copyright (c) 2011 - 2022, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/function/function_info.h>
#include <stingraykit/shared_ptr.h>
#include <stingraykit/Tuple.h>

namespace stingray
{

	namespace comparers
	{

		namespace Detail
		{

			struct ComparerType
			{
				STINGRAYKIT_ENUM_VALUES(Cmp, Relational, Equals, RelationalEquals);
				STINGRAYKIT_DECLARE_ENUM_CLASS(ComparerType);
			};

			template < ComparerType::Enum Type_, typename RetType_ = typename If<Type_ == ComparerType::Cmp, int, bool>::ValueT >
			struct ComparerInfoBase : public function_info<RetType_, UnspecifiedParamTypes>
			{
				static constexpr ComparerType::Enum CmpType = Type_;
			};

			template < typename Comparer >
			auto TestComparerHasCmpType(int) -> decltype(Comparer::CmpType, TrueType());
			template < typename Comparer >
			FalseType TestComparerHasCmpType(long);

		}


		using CmpComparerInfo = Detail::ComparerInfoBase<Detail::ComparerType::Cmp>;

		template < typename Comparer_, bool HasCmpType = decltype(Detail::TestComparerHasCmpType<Comparer_>(0))::Value >
		struct IsCmpComparer : public FalseType
		{ };

		template < typename Comparer_ >
		struct IsCmpComparer<Comparer_, true> : public integral_constant<bool, Comparer_::CmpType == Detail::ComparerType::Cmp && IsSame<typename function_info<Comparer_>::RetType, int>::Value>
		{ };


		using RelationalComparerInfo = Detail::ComparerInfoBase<Detail::ComparerType::Relational>;

		template < typename Comparer_, bool HasCmpType = decltype(Detail::TestComparerHasCmpType<Comparer_>(0))::Value >
		struct IsRelationalComparer : public FalseType
		{ };

		template < typename Comparer_ >
		struct IsRelationalComparer<Comparer_, true> : public integral_constant<bool, Comparer_::CmpType == Detail::ComparerType::Relational && IsSame<typename function_info<Comparer_>::RetType, bool>::Value>
		{ };

		template < typename T, bool HasCmpType >
		struct IsRelationalComparer<std::less<T>, HasCmpType> : public TrueType
		{ };

		template < typename T, bool HasCmpType >
		struct IsRelationalComparer<std::greater<T>, HasCmpType> : public TrueType
		{ };


		using EqualsComparerInfo = Detail::ComparerInfoBase<Detail::ComparerType::Equals>;

		template < typename Comparer_, bool HasCmpType = decltype(Detail::TestComparerHasCmpType<Comparer_>(0))::Value >
		struct IsEqualsComparer : public FalseType
		{ };

		template < typename Comparer_ >
		struct IsEqualsComparer<Comparer_, true> : public integral_constant<bool, Comparer_::CmpType == Detail::ComparerType::Equals && IsSame<typename function_info<Comparer_>::RetType, bool>::Value>
		{ };

		template < typename T, bool HasCmpType >
		struct IsEqualsComparer<std::equal_to<T>, HasCmpType> : public TrueType
		{ };

		template < typename T, bool HasCmpType >
		struct IsEqualsComparer<std::not_equal_to<T>, HasCmpType> : public TrueType
		{ };


		using RelationalEqualsComparerInfo = Detail::ComparerInfoBase<Detail::ComparerType::RelationalEquals>;

		template < typename Comparer_, bool HasCmpType = decltype(Detail::TestComparerHasCmpType<Comparer_>(0))::Value >
		struct IsRelationalEqualsComparer : public FalseType
		{ };

		template < typename Comparer_ >
		struct IsRelationalEqualsComparer<Comparer_, true> : public integral_constant<bool, Comparer_::CmpType == Detail::ComparerType::RelationalEquals && IsSame<typename function_info<Comparer_>::RetType, bool>::Value>
		{ };

		template < typename T, bool HasCmpType >
		struct IsRelationalEqualsComparer<std::less_equal<T>, HasCmpType> : public TrueType
		{ };

		template < typename T, bool HasCmpType >
		struct IsRelationalEqualsComparer<std::greater_equal<T>, HasCmpType> : public TrueType
		{ };


		template < typename Derived_ >
		struct CmpComparerBase : public CmpComparerInfo
		{
			template < typename Lhs, typename Rhs >
			typename EnableIf<!IsSharedPtr<Lhs>::Value || !IsSharedPtr<Rhs>::Value, int>::ValueT Compare(const Lhs& lhs, const Rhs& rhs) const
			{ return static_cast<const Derived_&>(*this).DoCompare(lhs, rhs); }

			template < typename Lhs, typename Rhs >
			typename EnableIf<IsSharedPtr<Lhs>::Value && IsSharedPtr<Rhs>::Value, int>::ValueT Compare(const Lhs& lhs, const Rhs& rhs) const
			{ return (lhs && rhs) ? Compare(*lhs, *rhs) : (lhs ? 1 : (rhs ? -1 : 0)); }

			template < typename Lhs, typename Rhs >
			int operator () (const Lhs& lhs, const Rhs& rhs) const
			{ return Compare(lhs, rhs); }
		};


		template < typename Derived_ >
		struct LessComparerBase : public RelationalComparerInfo
		{
			template < typename Lhs, typename Rhs >
			typename EnableIf<!IsSharedPtr<Lhs>::Value || !IsSharedPtr<Rhs>::Value, bool>::ValueT Compare(const Lhs& lhs, const Rhs& rhs) const
			{ return static_cast<const Derived_&>(*this).DoCompare(lhs, rhs); }

			template < typename Lhs, typename Rhs >
			typename EnableIf<IsSharedPtr<Lhs>::Value && IsSharedPtr<Rhs>::Value, bool>::ValueT Compare(const Lhs& lhs, const Rhs& rhs) const
			{ return (lhs && rhs) ? Compare(*lhs, *rhs) : (!lhs && rhs); }

			template < typename Lhs, typename Rhs >
			bool operator () (const Lhs& lhs, const Rhs& rhs) const
			{ return Compare(lhs, rhs); }
		};


		template < typename Derived_ >
		struct EqualsComparerBase : public EqualsComparerInfo
		{
			template < typename Lhs, typename Rhs >
			typename EnableIf<!IsSharedPtr<Lhs>::Value || !IsSharedPtr<Rhs>::Value, bool>::ValueT Compare(const Lhs& lhs, const Rhs& rhs) const
			{ return static_cast<const Derived_&>(*this).DoCompare(lhs, rhs); }

			template < typename Lhs, typename Rhs >
			typename EnableIf<IsSharedPtr<Lhs>::Value && IsSharedPtr<Rhs>::Value, bool>::ValueT Compare(const Lhs& lhs, const Rhs& rhs) const
			{ return (lhs && rhs) ? Compare(*lhs, *rhs) : (!lhs && !rhs); }

			template < typename Lhs, typename Rhs >
			bool operator () (const Lhs& lhs, const Rhs& rhs) const
			{ return Compare(lhs, rhs); }
		};


		template < typename Derived_ >
		struct GreaterComparerBase : public RelationalComparerInfo
		{
			template < typename Lhs, typename Rhs >
			typename EnableIf<!IsSharedPtr<Lhs>::Value || !IsSharedPtr<Rhs>::Value, bool>::ValueT Compare(const Lhs& lhs, const Rhs& rhs) const
			{ return static_cast<const Derived_&>(*this).DoCompare(lhs, rhs); }

			template < typename Lhs, typename Rhs >
			typename EnableIf<IsSharedPtr<Lhs>::Value && IsSharedPtr<Rhs>::Value, bool>::ValueT Compare(const Lhs& lhs, const Rhs& rhs) const
			{ return (lhs && rhs) ? Compare(*lhs, *rhs) : (lhs && !rhs); }

			template < typename Lhs, typename Rhs >
			bool operator () (const Lhs& lhs, const Rhs& rhs) const
			{ return Compare(lhs, rhs); }
		};


		struct Cmp : public CmpComparerBase<Cmp>
		{
		private:
			template < typename Lhs, typename Rhs >
			auto DoCompareImpl(const Lhs& lhs, const Rhs& rhs, int) const
					-> decltype(lhs.Compare(rhs), int())
			{ return lhs.Compare(rhs); }

			template < typename T >
			auto DoCompareImpl(const T& lhs, const T& rhs, long) const
					-> decltype(lhs < rhs, int())
			{
				if (lhs < rhs)
					return -1;
				if (rhs < lhs)
					return 1;
				return 0;
			}

		public:
			template < typename Lhs, typename Rhs >
			int DoCompare(const Lhs& lhs, const Rhs& rhs) const
			{ return DoCompareImpl(lhs, rhs, 0); }

			template < typename T >
			int DoCompare(const T& lhs, const T& rhs) const
			{
				if (&lhs == &rhs)
					return 0;

				return DoCompareImpl(lhs, rhs, 0);
			}
		};


		struct Less : public LessComparerBase<Less>
		{
		private:
			template < typename Lhs, typename Rhs >
			auto DoCompareImpl(const Lhs& lhs, const Rhs& rhs, int) const
					-> decltype(lhs.Compare(rhs), bool())
			{ return lhs.Compare(rhs) < 0; }

			template < typename T >
			auto DoCompareImpl(const T& lhs, const T& rhs, long) const
					-> decltype(lhs < rhs, bool())
			{ return lhs < rhs; }

		public:
			template < typename Lhs, typename Rhs >
			bool DoCompare(const Lhs& lhs, const Rhs& rhs) const
			{ return DoCompareImpl(lhs, rhs, 0); }

			template < typename T >
			bool DoCompare(const T& lhs, const T& rhs) const
			{
				if (&lhs == &rhs)
					return false;

				return DoCompareImpl(lhs, rhs, 0);
			}
		};


		struct Equals : public EqualsComparerBase<Equals>
		{
		private:
			template < typename Lhs, typename Rhs >
			auto DoCompareImpl(const Lhs& lhs, const Rhs& rhs, int) const
					-> decltype(lhs.Compare(rhs), bool())
			{ return lhs.Compare(rhs) == 0; }

			template < typename T >
			auto DoCompareImpl(const T& lhs, const T& rhs, long) const
					-> decltype(lhs == rhs, bool())
			{ return lhs == rhs; }

		public:
			template < typename Lhs, typename Rhs >
			bool DoCompare(const Lhs& lhs, const Rhs& rhs) const
			{ return DoCompareImpl(lhs, rhs, 0); }

			template < typename T >
			bool DoCompare(const T& lhs, const T& rhs) const
			{
				if (&lhs == &rhs)
					return true;

				return DoCompareImpl(lhs, rhs, 0);
			}
		};


		struct Greater : public GreaterComparerBase<Greater>
		{
		private:
			template < typename Lhs, typename Rhs >
			auto DoCompareImpl(const Lhs& lhs, const Rhs& rhs, int) const
					-> decltype(lhs.Compare(rhs), bool())
			{ return lhs.Compare(rhs) > 0; }

			template < typename T >
			auto DoCompareImpl(const T& lhs, const T& rhs, long) const
					-> decltype(rhs < lhs, bool())
			{ return rhs < lhs; }

		public:
			template < typename Lhs, typename Rhs >
			bool DoCompare(const Lhs& lhs, const Rhs& rhs) const
			{ return DoCompareImpl(lhs, rhs, 0); }

			template < typename T >
			bool DoCompare(const T& lhs, const T& rhs) const
			{
				if (&lhs == &rhs)
					return false;

				return DoCompareImpl(lhs, rhs, 0);
			}
		};


		template < typename CmpComparer_ >
		struct CmpToLess : public RelationalComparerInfo
		{
			static_assert(IsCmpComparer<CmpComparer_>::Value, "Expected Cmp comparer");

		private:
			CmpComparer_	_comparer;

		public:
			CmpToLess()
			{ }

			CmpToLess(const CmpComparer_& comparer) : _comparer(comparer)
			{ }

			template < typename Lhs, typename Rhs >
			bool operator () (const Lhs& lhs, const Rhs& rhs) const
			{ return _comparer(lhs, rhs) < 0; }
		};


		template < typename CmpComparer_ >
		struct CmpToGreater : public RelationalComparerInfo
		{
			static_assert(IsCmpComparer<CmpComparer_>::Value, "Expected Cmp comparer");

		private:
			CmpComparer_	_comparer;

		public:
			CmpToGreater()
			{ }

			CmpToGreater(const CmpComparer_& comparer) : _comparer(comparer)
			{ }

			template < typename Lhs, typename Rhs >
			bool operator () (const Lhs& lhs, const Rhs& rhs) const
			{ return _comparer(lhs, rhs) > 0; }
		};


		template < typename CmpComparer_ >
		struct CmpToEquals : public EqualsComparerInfo
		{
			static_assert(IsCmpComparer<CmpComparer_>::Value, "Expected Cmp comparer");

		private:
			CmpComparer_	_comparer;

		public:
			CmpToEquals()
			{ }

			CmpToEquals(const CmpComparer_& comparer) : _comparer(comparer)
			{ }

			template < typename Lhs, typename Rhs >
			bool operator () (const Lhs& lhs, const Rhs& rhs) const
			{ return _comparer(lhs, rhs) == 0; }
		};


		template < typename LessComparer_ >
		struct LessToEquals : public EqualsComparerInfo
		{
		private:
			LessComparer_	_lessComparer;

		public:
			LessToEquals()
			{ }

			LessToEquals(const LessComparer_& comparer) : _lessComparer(comparer)
			{ }

			template < typename Lhs, typename Rhs >
			bool operator () (const Lhs& lhs, const Rhs& rhs) const
			{ return !_lessComparer(lhs, rhs) && !_lessComparer(rhs, lhs); }
		};


		template < typename LessComparer_ >
		struct LessToCmp : public CmpComparerInfo
		{
		private:
			LessComparer_	_lessComparer;

		public:
			LessToCmp()
			{ }

			LessToCmp(const LessComparer_& comparer) : _lessComparer(comparer)
			{ }

			template < typename Lhs, typename Rhs >
			int operator () (const Lhs& lhs, const Rhs& rhs) const
			{ return _lessComparer(lhs, rhs) ? -1 : (_lessComparer(rhs, lhs) ? 1 : 0); }
		};


#define STINGRAYKIT_DECLARE_COMPARERS(ClassName) \
		using ClassName##Less = stingray::comparers::CmpToLess<ClassName##Cmp>; \
		using ClassName##Equals = stingray::comparers::CmpToEquals<ClassName##Cmp>; \
		using ClassName##Greater = stingray::comparers::CmpToGreater<ClassName##Cmp>

#define STINGRAYKIT_DECLARE_TEMPLATE_COMPARERS(ClassName, TemplateDecl, TemplateUsage) \
		template < TemplateDecl > \
		using ClassName##Less = stingray::comparers::CmpToLess<ClassName##Cmp<TemplateUsage>>; \
		template < TemplateDecl > \
		using ClassName##Equals = stingray::comparers::CmpToEquals<ClassName##Cmp<TemplateUsage>>; \
		template < TemplateDecl > \
		using ClassName##Greater = stingray::comparers::CmpToGreater<ClassName##Cmp<TemplateUsage>>

	}


	struct OwnerCmp : public comparers::CmpComparerInfo
	{
		template < typename Lhs, typename Rhs >
		int operator () (const Lhs& lhs, const Rhs& rhs) const
		{ return lhs.owner_before(rhs) ? -1 : (rhs.owner_before(lhs) ? 1 : 0); }
	};
	STINGRAYKIT_DECLARE_COMPARERS(Owner);


	struct PointerCmp : public comparers::CmpComparerInfo
	{
		template < typename Lhs, typename Rhs >
		int operator () (const shared_ptr<Lhs>& lhs, const shared_ptr<Rhs>& rhs) const
		{ return comparers::Cmp()(lhs.get(), rhs.get()); }
	};
	STINGRAYKIT_DECLARE_COMPARERS(Pointer);


	template < typename CompareFunc >
	class TupleCmp : public comparers::CmpComparerInfo
	{
	private:
		CompareFunc		_compareFunc;

	public:
		explicit TupleCmp(const CompareFunc& compareFunc = CompareFunc()) : _compareFunc(compareFunc) { }

		int operator () (const Tuple<TypeList<>>&, const Tuple<TypeList<>>&) const
		{ return 0; }

		template < typename T >
		int operator () (const Tuple<T>& lhs, const Tuple<T>& rhs) const
		{
			if (const int result = _compareFunc(lhs.GetHead(), rhs.GetHead()))
				return result;
			return TupleCmp<CompareFunc>(_compareFunc)(lhs.GetTail(), rhs.GetTail());
		}
	};
	template < typename CompareFunc >
	using TupleLess = comparers::CmpToLess<TupleCmp<CompareFunc>>;
	template < typename CompareFunc >
	using TupleGreater = comparers::CmpToGreater<TupleCmp<CompareFunc>>;


	struct TupleEquals : public comparers::EqualsComparerInfo
	{
		bool operator () (const Tuple<TypeList<>>&, const Tuple<TypeList<>>&) const
		{ return true; }

		template < typename T >
		bool operator () (const Tuple<T>& lhs, const Tuple<T>& rhs) const
		{
			if (lhs.GetHead() != rhs.GetHead())
				return false;
			return TupleEquals()(lhs.GetTail(), rhs.GetTail());
		}
	};

}

#endif
