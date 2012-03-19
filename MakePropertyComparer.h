#ifndef __GS_DVRLIB_TOOLKIT_MAKEPROPERTYCOMPARER_H__
#define __GS_DVRLIB_TOOLKIT_MAKEPROPERTYCOMPARER_H__


#include <stingray/toolkit/IComparer.h>
#include <stingray/toolkit/MetaProgramming.h>


namespace stingray
{

	/*! \cond GS_INTERNAL */

	template < typename ComparableClassType, typename MethodClassType, typename PropertyType >
	class PropertyComparer : public virtual IComparer<ComparableClassType>
	{
	public:
		typedef PropertyType (MethodClassType::*PropertyGetter)() const;

	private:
		PropertyGetter	_getter;

	public:
		PropertyComparer(const PropertyGetter& getter)
			: _getter(getter)
		{ }

		virtual int Compare(const shared_ptr<ComparableClassType>& left, const shared_ptr<ComparableClassType>& right) const
		{
			if (left == 0)
			{
				if (right == 0) 
					return 0;
				return -1;
			}

			if (right == 0)
				return 1;

			PropertyType l = (left.get()->*_getter)();
			PropertyType r = (right.get()->*_getter)();

			if (l < r)			return -1;
			else if (r < l)		return 1;
			else				return 0;
		}
	};


	/*! \endcond */

	template < typename ClassType, typename PropertyType >
	shared_ptr< IComparer<ClassType> > MakePropertyComparer(PropertyType (ClassType::*propertyGetter)() const)
	{ return make_shared<PropertyComparer<ClassType, ClassType, PropertyType> >(propertyGetter); }

	template < typename ComparableClassType, typename MethodClassType, typename PropertyType >
	shared_ptr< IComparer<typename EnableIf<!SameType<ComparableClassType, MethodClassType>::Value, ComparableClassType>::ValueT> > 
		MakePropertyComparer(PropertyType (MethodClassType::*propertyGetter)() const, const Dummy& dummy = Dummy())
	{ return make_shared<PropertyComparer<ComparableClassType, MethodClassType, PropertyType> >(propertyGetter); }


}


#endif
