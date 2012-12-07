#ifndef STINGRAY_TOOLKIT_LAZYPROPERTY_H
#define STINGRAY_TOOLKIT_LAZYPROPERTY_H

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
