#ifndef STINGRAYKIT_COMPARE_COMPARERS_H
#define STINGRAYKIT_COMPARE_COMPARERS_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/function/function_info.h>
#include <stingraykit/shared_ptr.h>

namespace stingray
{

	namespace comparers
	{

		template<typename Derived_>
		struct CmpComparerBase : public function_info<int, UnspecifiedParamTypes>
		{
			template <typename Lhs, typename Rhs>
			typename EnableIf<!IsSharedPtr<Lhs>::Value || !IsSharedPtr<Rhs>::Value, int>::ValueT Compare(const Lhs& lhs, const Rhs& rhs) const
			{ return static_cast<const Derived_&>(*this).DoCompare(lhs, rhs); }

			template <typename Lhs, typename Rhs>
			typename EnableIf<IsSharedPtr<Lhs>::Value && IsSharedPtr<Rhs>::Value, int>::ValueT Compare(const Lhs& lhs, const Rhs& rhs, const Dummy& dummy = Dummy()) const
			{ return (lhs && rhs) ? Compare(*lhs, *rhs) : (lhs ? 1 : (rhs ? -1 : 0)); }

			template < typename Lhs, typename Rhs >
			int operator () (const Lhs& lhs, const Rhs& rhs) const
			{ return Compare(lhs, rhs); }
		};


		template<typename Derived_>
		struct LessComparerBase : public function_info<bool, UnspecifiedParamTypes>
		{
			template <typename Lhs, typename Rhs>
			typename EnableIf<!IsSharedPtr<Lhs>::Value || !IsSharedPtr<Rhs>::Value, bool>::ValueT Compare(const Lhs& lhs, const Rhs& rhs) const
			{ return static_cast<const Derived_&>(*this).DoCompare(lhs, rhs); }

			template <typename Lhs, typename Rhs>
			typename EnableIf<IsSharedPtr<Lhs>::Value && IsSharedPtr<Rhs>::Value, bool>::ValueT Compare(const Lhs& lhs, const Rhs& rhs, const Dummy& dummy = Dummy()) const
			{ return (lhs && rhs) ? Compare(*lhs, *rhs) : (!lhs && rhs); }

			template < typename Lhs, typename Rhs >
			bool operator () (const Lhs& lhs, const Rhs& rhs) const
			{ return Compare(lhs, rhs); }
		};


		template<typename Derived_>
		struct EqualsComparerBase : public function_info<bool, UnspecifiedParamTypes>
		{
			template <typename Lhs, typename Rhs>
			typename EnableIf<!IsSharedPtr<Lhs>::Value || !IsSharedPtr<Rhs>::Value, bool>::ValueT Compare(const Lhs& lhs, const Rhs& rhs) const
			{ return static_cast<const Derived_&>(*this).DoCompare(lhs, rhs); }

			template <typename Lhs, typename Rhs>
			typename EnableIf<IsSharedPtr<Lhs>::Value && IsSharedPtr<Rhs>::Value, bool>::ValueT Compare(const Lhs& lhs, const Rhs& rhs, const Dummy& dummy = Dummy()) const
			{ return (lhs && rhs) ? Compare(*lhs, *rhs) : (!lhs && !rhs); }

			template < typename Lhs, typename Rhs >
			bool operator () (const Lhs& lhs, const Rhs& rhs) const
			{ return Compare(lhs, rhs); }
		};


		template<typename Derived_>
		struct GreaterComparerBase : public function_info<bool, UnspecifiedParamTypes>
		{
			template <typename Lhs, typename Rhs>
			typename EnableIf<!IsSharedPtr<Lhs>::Value || !IsSharedPtr<Rhs>::Value, bool>::ValueT Compare(const Lhs& lhs, const Rhs& rhs) const
			{ return static_cast<const Derived_&>(*this).DoCompare(lhs, rhs); }

			template <typename Lhs, typename Rhs>
			typename EnableIf<IsSharedPtr<Lhs>::Value && IsSharedPtr<Rhs>::Value, bool>::ValueT Compare(const Lhs& lhs, const Rhs& rhs, const Dummy& dummy = Dummy()) const
			{ return (lhs && rhs) ? Compare(*lhs, *rhs) : (lhs && !rhs); }

			template < typename Lhs, typename Rhs >
			bool operator () (const Lhs& lhs, const Rhs& rhs) const
			{ return Compare(lhs, rhs); }
		};


		namespace Detail
		{
			STINGRAYKIT_DECLARE_METHOD_CHECK(Compare);
		}


		struct Cmp : public CmpComparerBase<Cmp>
		{
			template<typename T>
			typename EnableIf<Detail::HasMethod_Compare<T>::Value, int>::ValueT DoCompare(const T& lhs, const T& rhs) const
			{
				if (&lhs == &rhs)
					return 0;

				return lhs.Compare(rhs);
			}

			template<typename T>
			typename EnableIf<!Detail::HasMethod_Compare<T>::Value, int>::ValueT DoCompare(const T& lhs, const T& rhs, const Dummy& dummy = Dummy()) const
			{
				if (&lhs == &rhs)
					return 0;

				if (lhs < rhs)
					return -1;
				if (rhs < lhs)
					return 1;
				return 0;
			}
		};


		struct Less : public LessComparerBase<Less>
		{
			template<typename T>
			typename EnableIf<Detail::HasMethod_Compare<T>::Value, bool>::ValueT DoCompare(const T& lhs, const T& rhs) const
			{
				if (&lhs == &rhs)
					return false;

				return lhs.Compare(rhs) < 0;
			}

			template<typename T>
			typename EnableIf<!Detail::HasMethod_Compare<T>::Value, bool>::ValueT DoCompare(const T& lhs, const T& rhs, const Dummy& dummy = Dummy()) const
			{
				if (&lhs == &rhs)
					return false;

				return lhs < rhs;
			}
		};


		struct Equals : public EqualsComparerBase<Equals>
		{
			template<typename T>
			typename EnableIf<Detail::HasMethod_Compare<T>::Value, bool>::ValueT DoCompare(const T& lhs, const T& rhs) const
			{
				if (&lhs == &rhs)
					return true;

				return lhs.Compare(rhs) == 0;
			}

			template<typename T>
			typename EnableIf<!Detail::HasMethod_Compare<T>::Value, bool>::ValueT DoCompare(const T& lhs, const T& rhs, const Dummy& dummy = Dummy()) const
			{
				if (&lhs == &rhs)
					return true;

				return lhs == rhs;
			}
		};


		struct Greater : public GreaterComparerBase<Greater>
		{
			template<typename T>
			typename EnableIf<Detail::HasMethod_Compare<T>::Value, bool>::ValueT DoCompare(const T& lhs, const T& rhs) const
			{
				if (&lhs == &rhs)
					return false;

				return lhs.Compare(rhs) > 0;
			}

			template<typename T>
			typename EnableIf<!Detail::HasMethod_Compare<T>::Value, bool>::ValueT DoCompare(const T& lhs, const T& rhs, const Dummy& dummy = Dummy()) const
			{
				if (&lhs == &rhs)
					return false;

				return rhs < lhs;
			}
		};


		template<typename CmpComparer_>
		struct CmpToLess : public function_info<bool, UnspecifiedParamTypes>
		{
		private:
			CmpComparer_	_comparer;

		public:
			CmpToLess()
			{ }

			CmpToLess(const CmpComparer_& comparer) : _comparer(comparer)
			{ }

			template <typename Lhs, typename Rhs>
			bool operator () (const Lhs& lhs, const Rhs& rhs) const
			{
				CompileTimeAssert<IsSame<typename function_info<CmpComparer_>::RetType, int>::Value> ErrorExpectedCmpComparer;
				(void)ErrorExpectedCmpComparer;

				return _comparer(lhs, rhs) < 0;
			}
		};


		template<typename CmpComparer_>
		struct CmpToGreater : public function_info<bool, UnspecifiedParamTypes>
		{
		private:
			CmpComparer_	_comparer;

		public:
			CmpToGreater()
			{ }

			CmpToGreater(const CmpComparer_& comparer) : _comparer(comparer)
			{ }

			template <typename Lhs, typename Rhs>
			bool operator () (const Lhs& lhs, const Rhs& rhs) const
			{
				CompileTimeAssert<IsSame<typename function_info<CmpComparer_>::RetType, int>::Value> ErrorExpectedCmpComparer;
				(void)ErrorExpectedCmpComparer;

				return _comparer(lhs, rhs) > 0;
			}
		};


		template<typename CmpComparer_>
		struct CmpToEquals : public function_info<bool, UnspecifiedParamTypes>
		{
		private:
			CmpComparer_	_comparer;

		public:
			CmpToEquals()
			{ }

			CmpToEquals(const CmpComparer_& comparer) : _comparer(comparer)
			{ }

			template <typename Lhs, typename Rhs>
			bool operator () (const Lhs& lhs, const Rhs& rhs) const
			{
				CompileTimeAssert<IsSame<typename function_info<CmpComparer_>::RetType, int>::Value> ErrorExpectedCmpComparer;
				(void)ErrorExpectedCmpComparer;

				return _comparer(lhs, rhs) == 0;
			}
		};


		template<typename LessComparer_>
		struct LessToEquals : public function_info<bool, UnspecifiedParamTypes>
		{
		private:
			LessComparer_	_lessComparer;

		public:
			LessToEquals()
			{ }

			LessToEquals(const LessComparer_& comparer) : _lessComparer(comparer)
			{ }

			template <typename Lhs, typename Rhs>
			bool operator () (const Lhs& lhs, const Rhs& rhs) const
			{ return !_lessComparer(lhs, rhs) && !_lessComparer(rhs, lhs); }
		};


		template<typename LessComparer_>
		struct LessToCmp : public function_info<int, UnspecifiedParamTypes>
		{
		private:
			LessComparer_	_lessComparer;

		public:
			LessToCmp()
			{ }

			LessToCmp(const LessComparer_& comparer) : _lessComparer(comparer)
			{ }

			template <typename Lhs, typename Rhs>
			int operator () (const Lhs& lhs, const Rhs& rhs) const
			{ return _lessComparer(lhs, rhs) ? -1 : (_lessComparer(rhs, lhs) ? 1 : 0); }
		};


#define STINGRAYKIT_DECLARE_COMPARERS(ClassName) \
	typedef stingray::comparers::CmpToLess<ClassName##Cmp> ClassName##Less; \
	typedef stingray::comparers::CmpToEquals<ClassName##Cmp> ClassName##Equals; \
	typedef stingray::comparers::CmpToGreater<ClassName##Cmp> ClassName##Greater;

	}


	struct OwnerCmp : public function_info<int, UnspecifiedParamTypes>
	{
		template < typename Lhs, typename Rhs >
		int operator () (const Lhs& lhs, const Rhs& rhs) const
		{ return lhs.owner_before(rhs) ? -1 : (rhs.owner_before(lhs) ? 1 : 0); }
	};
	STINGRAYKIT_DECLARE_COMPARERS(Owner);


	struct PointerCmp : public function_info<int, UnspecifiedParamTypes>
	{
		template < typename Lhs, typename Rhs >
		int operator () (const shared_ptr<Lhs>& lhs, const shared_ptr<Rhs>& rhs) const
		{ return comparers::Cmp()(lhs.get(), rhs.get()); }
	};
	STINGRAYKIT_DECLARE_COMPARERS(Pointer);


	template < typename CompareFunc >
	class TupleCmp : public function_info<int, UnspecifiedParamTypes>
	{
	private:
		CompareFunc		_compareFunc;

	public:
		explicit TupleCmp(const CompareFunc& compareFunc = comparers::Cmp()) : _compareFunc(compareFunc) { }

		int operator () (const Tuple<TypeList<>::type>&, const Tuple<TypeList<>::type>&) const
		{ return 0; }

		template < typename T >
		int operator () (const Tuple<T>& lhs, const Tuple<T>& rhs) const
		{
			if (const int result = _compareFunc(lhs.GetHead(), rhs.GetHead()))
				return result;
			return TupleCmp<CompareFunc>(_compareFunc)(lhs.GetTail(), rhs.GetTail());
		}
	};


	struct TupleEquals : public function_info<bool, UnspecifiedParamTypes>
	{
		bool operator () (const Tuple<TypeList<>::type>&, const Tuple<TypeList<>::type>&) const
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
