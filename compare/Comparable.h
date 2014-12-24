#ifndef STINGRAY_TOOLKIT_COMPARE_COMPARABLE_H
#define STINGRAY_TOOLKIT_COMPARE_COMPARABLE_H

#include <typeinfo>

#include <stingray/toolkit/compare/comparers.h>
#include <stingray/toolkit/shared_ptr.h>

namespace stingray
{

	struct IComparable
	{
		virtual ~IComparable() {}
		virtual int Compare(const IComparable &other) const = 0;
	};


	template<typename T>
	struct Comparable : public virtual IComparable
	{
		virtual int Compare(const IComparable &other) const
		{
			const std::type_info &my_type = typeid(*this);
			const std::type_info &other_type = typeid(other);

			if (my_type != other_type)
				return my_type.before(other_type) ? -1 : 1;

			//avoiding dynamic_cast here
			const char * this_ptr = reinterpret_cast<const char *>(this);
			const char * this_icomparable = reinterpret_cast<const char *>(static_cast<const IComparable *>(this));
			ptrdiff_t delta = this_ptr - this_icomparable; //distance between Comparable and IComparable for this type.
			const char * other_icomparable = reinterpret_cast<const char *>(&other);
			const Comparable<T> *other_ptr = reinterpret_cast<const Comparable<T> *>(other_icomparable + delta);

			return this->DoCompare(*static_cast<const T*>(other_ptr));
		}

	protected:
		virtual int DoCompare(const T &other) const = 0;
	};


	struct ComparableCmp : public comparers::CmpComparerBase<ComparableCmp>
	{
		int DoCompare(const IComparable& l, const IComparable& r) const		{ return l.Compare(r); }
	};


	struct ComparableLess : public comparers::LessComparerBase<ComparableLess>
	{
		bool DoCompare(const IComparable& l, const IComparable& r) const	{ return l.Compare(r) < 0; }
	};


	struct ComparableEquals : public comparers::EqualsComparerBase<ComparableEquals>
	{
		bool DoCompare(const IComparable& l, const IComparable& r) const	{ return l.Compare(r) == 0; }
	};

}

#endif
