#ifndef __TOOLKIT_SAFE_BOOL_H__
#define __TOOLKIT_SAFE_BOOL_H__

#include <stingray/toolkit/toolkit.h>

namespace stingray 
{

class safe_bool_base
{
protected:
	void this_type_does_not_support_comparisons() const {}
	
	FORCE_INLINE safe_bool_base() {}
	FORCE_INLINE ~safe_bool_base() {}

	FORCE_INLINE safe_bool_base(const safe_bool_base&) {}
	FORCE_INLINE safe_bool_base& operator=(const safe_bool_base&) {return *this;}
};


template <typename T = void> class safe_bool : private safe_bool_base {
	typedef void (safe_bool::*bool_type)() const;
public:
	FORCE_INLINE operator bool_type() const
	{
		return (static_cast<const T*>(this))->boolean_test()? &safe_bool_base::this_type_does_not_support_comparisons : 0;
	}

protected:
	FORCE_INLINE ~safe_bool() {}
};

}

#endif
