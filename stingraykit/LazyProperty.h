#ifndef STINGRAYKIT_LAZYPROPERTY_H
#define STINGRAYKIT_LAZYPROPERTY_H

#include <stingraykit/shared_ptr.h>

namespace stingray {

#define STINGRAYKIT_LAZY_PROPERTY(PropertyType, PropertyName) \
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
