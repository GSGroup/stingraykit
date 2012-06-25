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

}


#endif
