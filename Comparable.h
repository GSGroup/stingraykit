#ifndef COMPARABLE_H
#define	COMPARABLE_H

#include <typeinfo>
#include <dvrlib/toolkit/shared_ptr.h>

namespace dvrlib
{
	struct IComparable;
	TOOLKIT_DECLARE_PTR(IComparable);
	struct IComparable
	{
		virtual ~IComparable() {}
		virtual int Compare(const IComparablePtr &other) const = 0;
	};

	template<typename T>
	struct Comparable : public virtual IComparable
	{
		virtual int Compare(const IComparablePtr &other) const
		{
			if (!other)
				return 1;

			const std::type_info &my_type = typeid(T);
			const std::type_info &other_type = typeid(*other);

			if (my_type == other_type)
				return this->Compare(dynamic_pointer_cast<T>(other));

			return my_type.before(other_type)? -1 : 1;
		}

		virtual int Compare(const shared_ptr<T> &other) const = 0;
	};

	template<typename T>
	struct ComparableLess
	{
		bool operator()(const shared_ptr<T>& arg1, const shared_ptr<T>& arg2) const
		{
			if (!arg2)
				return false;

			return arg1? arg1->Compare(arg2) < 0 : true;
		}
	};

	template<typename T>
	struct ComparableEquals : std::binary_function<const shared_ptr<T>&, const shared_ptr<T>&, bool>
	{
		bool operator()(const shared_ptr<T>& lhs, const shared_ptr<T>& rhs) const
		{
			return lhs ? lhs->Compare(rhs) == 0 : (bool)!rhs;
		}
	};

}

#endif
