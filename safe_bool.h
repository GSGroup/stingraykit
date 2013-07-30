#ifndef STINGRAY_TOOLKIT_SAFE_BOOL_H
#define STINGRAY_TOOLKIT_SAFE_BOOL_H

#include <stingray/toolkit/toolkit.h>

namespace stingray 
{

class safe_bool_base
{
protected:
	void this_type_does_not_support_comparisons() const {}
	
	inline safe_bool_base() {}
	inline ~safe_bool_base() {}

	inline safe_bool_base(const safe_bool_base&) {}
	inline safe_bool_base& operator=(const safe_bool_base&) {return *this;}
};


template <typename T = void> class safe_bool : private safe_bool_base {
	typedef void (safe_bool::*bool_type)() const;
public:
	inline operator bool_type() const
	{
		return (static_cast<const T*>(this))->boolean_test()? &safe_bool::this_type_does_not_support_comparisons : 0;
	}

protected:
	inline ~safe_bool() {}
};

}

#endif
