#ifndef STINGRAY_TOOLKIT_COMPARERS_H
#define STINGRAY_TOOLKIT_COMPARERS_H


#include <stingray/toolkit/exception.h>
#include <stingray/toolkit/function/function_info.h>
#include <stingray/toolkit/shared_ptr.h>


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
			TOOLKIT_DECLARE_METHOD_CHECK(Compare);
		}


		struct Cmp : public CmpComparerBase<Cmp>
		{
			template<typename T>
			typename EnableIf<Detail::HasMethod_Compare<T>::Value, int>::ValueT DoCompare(const T& lhs, const T& rhs) const
			{ return lhs.Compare(rhs); }

			template<typename T>
			typename EnableIf<!Detail::HasMethod_Compare<T>::Value, int>::ValueT DoCompare(const T& lhs, const T& rhs, const Dummy& dummy = Dummy()) const
			{
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
			{ return lhs.Compare(rhs) < 0; }

			template<typename T>
			typename EnableIf<!Detail::HasMethod_Compare<T>::Value, bool>::ValueT DoCompare(const T& lhs, const T& rhs, const Dummy& dummy = Dummy()) const
			{ return lhs < rhs; }
		};


		struct Equals : public EqualsComparerBase<Equals>
		{
			template<typename T>
			typename EnableIf<Detail::HasMethod_Compare<T>::Value, bool>::ValueT DoCompare(const T& lhs, const T& rhs) const
			{ return lhs.Compare(rhs) == 0; }

			template<typename T>
			typename EnableIf<!Detail::HasMethod_Compare<T>::Value, bool>::ValueT DoCompare(const T& lhs, const T& rhs, const Dummy& dummy = Dummy()) const
			{ return lhs == rhs; }
		};


		struct Greater : public GreaterComparerBase<Greater>
		{
			template<typename T>
			typename EnableIf<Detail::HasMethod_Compare<T>::Value, bool>::ValueT DoCompare(const T& lhs, const T& rhs) const
			{ return lhs.Compare(rhs) > 0; }

			template<typename T>
			typename EnableIf<!Detail::HasMethod_Compare<T>::Value, bool>::ValueT DoCompare(const T& lhs, const T& rhs, const Dummy& dummy = Dummy()) const
			{ return rhs < lhs; }
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
			{ return _comparer(lhs, rhs) < 0; }
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
			{ return _comparer(lhs, rhs) > 0; }
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
			{ return _comparer(lhs, rhs) == 0; }
		};


#define TOOLKIT_DECLARE_COMPARERS(ClassName) \
	typedef stingray::comparers::CmpToLess<ClassName##Cmp> ClassName##Less; \
	typedef stingray::comparers::CmpToEquals<ClassName##Cmp> ClassName##Equals; \
	typedef stingray::comparers::CmpToGreater<ClassName##Cmp> ClassName##Greater;

	}


	template < typename less_comparer >
	class less_to_equals : public function_info<less_comparer>
	{
	private:
		less_comparer	_less;

	public:
		less_to_equals() { }
		less_to_equals(const less_comparer& less) : _less(less) { }

		template < typename T, typename U >
		bool operator () (const T& l, const U& r) const
		{ return !_less(l, r) && !_less(r, l); }
	};


	template < typename T >
	struct owner_equals : public less_to_equals<owner_less<T> >
	{ };


	template < typename less_comparer >
	class less_to_cmp : public function_info<int, UnspecifiedParamTypes>
	{
	private:
		less_comparer	_less;

	public:
		less_to_cmp() { }
		less_to_cmp(const less_comparer& less) : _less(less) { }

		template < typename T, typename U >
		int operator () (const T& l, const U& r) const
		{ return _less(l, r) ? -1 : (_less(r, l) ? 1 : 0); }
	};


	template < typename T >
	struct owner_cmp : public less_to_cmp<owner_less<T> >
	{ };


	template<typename ItemComparer>
	struct CollectionComparer : public function_info<ItemComparer>
	{
		template < typename T >
		int operator () (const T& lhs, const T& rhs) const
		{
			typename T::const_iterator first1 = lhs.begin(), last1 = lhs.end();
			typename T::const_iterator first2 = rhs.begin(), last2 = rhs.end();
			for (; first1 != last1; ++first1, ++first2)
			{
				if (first2 == last2)
					return -1;

				int item_result = ItemComparer()(*first1, *first2);
				if (item_result != 0)
					return item_result;
			}
			return first2 == last2 ? 0 : 1;
		}
	};


}


#endif
