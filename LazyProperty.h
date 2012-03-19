#ifndef __GS_DVRLIB_TOOLKIT_LAZYPROPERTY_H__
#define	__GS_DVRLIB_TOOLKIT_LAZYPROPERTY_H__

#include <stingray/toolkit/shared_ptr.h>

namespace stingray {

#define TOOLKIT_LAZY_PROPERTY(PropertyType, PropertyName) \
	public: \
		const PropertyType& Get##PropertyName() const \
		{ \
			if (!_Detail##PropertyName) \
				_Detail##PropertyName = DetailGet##PropertyName(); \
			return *_Detail##PropertyName; \
		} \
	private: \
		mutable shared_ptr<PropertyType> _Detail##PropertyName; \
		shared_ptr<PropertyType> DetailGet##PropertyName() const

}

#endif
