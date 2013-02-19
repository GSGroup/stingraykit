#ifndef STINGRAY_TOOLKIT_COMPARERS_H
#define STINGRAY_TOOLKIT_COMPARERS_H


#include <stingray/toolkit/function_info.h>


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


	struct CompareMethodComparer
	{
		template < typename PtrType >
		int operator () (const PtrType& l, const PtrType& r) const
		{ return l ? l->Compare(r) : (r ? -1 : 0); }
	};


	struct StandardOperatorsComparer
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


	struct DereferencerComparer
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


}


#endif
