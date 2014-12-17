#ifndef STINGRAY_TOOLKIT_COMPARERS_H
#define STINGRAY_TOOLKIT_COMPARERS_H


#include <stingray/toolkit/MemberListComparer.h>
#include <stingray/toolkit/exception.h>
#include <stingray/toolkit/function/function_info.h>
#include <stingray/toolkit/shared_ptr.h>


namespace stingray
{

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
		bool operator () (const T& l, const U& r) const
		{ return _less(l, r) ? -1 : (_less(r, l) ? 1 : 0); }
	};


	template < typename T >
	struct owner_cmp : public less_to_cmp<owner_less<T> >
	{ };


	struct CompareMethodComparer : public function_info<int, UnspecifiedParamTypes>
	{
		template < typename PtrType >
		typename EnableIf<(IsPointer<PtrType>::Value || IsSharedPtr<PtrType>::Value), int>::ValueT   operator () (const PtrType& l, const PtrType& r) const
		{ return l ? (r? l->Compare(*r): 1) : (r ? -1 : 0); }

		template < typename Type >
		typename EnableIf<!(IsPointer<Type>::Value || IsSharedPtr<Type>::Value), int>::ValueT   operator () (const Type& l, const Type& r) const
		{ return l.Compare(r); }
	};


	template < typename CustomLess >
	struct CustomLessComparer : public function_info<int, UnspecifiedParamTypes>
	{
	private:
		CustomLess	_customLess;

	public:
		CustomLessComparer(const CustomLess& customLess = CustomLess())
			: _customLess(customLess)
		{ }

		template < typename T >
		int operator () (const T& lhs, const T& rhs) const
		{
			if (_customLess(lhs, rhs))
				return -1;
			if (_customLess(rhs, lhs))
				return 1;
			return 0;
		}
	};


	struct StandardOperatorsComparer : public function_info<int, UnspecifiedParamTypes>
	{
		template < typename T >
		int operator () (const T& lhs, const T& rhs) const
		{
			if (lhs < rhs)
				return -1;
			if (rhs < lhs)
				return 1;
			return 0;
		}
	};


	struct DereferencerComparer : public function_info<int, UnspecifiedParamTypes>
	{
		template < typename PtrType >
		int operator () (const PtrType& l, const PtrType& r) const
		{
			TOOLKIT_REQUIRE_NOT_NULL(l);
			TOOLKIT_REQUIRE_NOT_NULL(r);
			if (*l < *r)
				return -1;
			if (*r < *l)
				return 1;
			return 0;
		}
	};


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
