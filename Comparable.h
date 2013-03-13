#ifndef STINGRAY_TOOLKIT_COMPARABLE_H
#define STINGRAY_TOOLKIT_COMPARABLE_H

#include <typeinfo>

#include <stingray/toolkit/comparers.h>
#include <stingray/toolkit/shared_ptr.h>

namespace stingray
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

			const std::type_info &my_type = typeid(*this);
			const std::type_info &other_type = typeid(*other);

			if (my_type == other_type)
				return this->Compare(dynamic_pointer_cast<T>(other));

			return my_type.before(other_type)? -1 : 1;
		}

		virtual int Compare(const shared_ptr<T> &other) const = 0;
	};

	template<typename T = IComparable>
	struct ComparableLess
	{
		bool operator()(const shared_ptr<T>& arg1, const shared_ptr<T>& arg2) const
		{
			if (!arg2)
				return false;

			return arg1? arg1->Compare(arg2) < 0 : true;
		}
	};

	template<typename T = IComparable>
	struct ComparableEquals : std::binary_function<const shared_ptr<T>&, const shared_ptr<T>&, bool>
	{
		bool operator()(const shared_ptr<T>& lhs, const shared_ptr<T>& rhs) const
		{
			return lhs ? lhs->Compare(rhs) == 0 : (bool)!rhs;
		}
	};


	struct IComparableNew
	{
		virtual ~IComparableNew() {}
		virtual int Compare(const IComparableNew &other) const = 0;
	};


	template<typename T>
	struct ComparableNew : public virtual IComparableNew
	{
		virtual int Compare(const IComparableNew &other) const
		{
			const std::type_info &my_type = typeid(*this);
			const std::type_info &other_type = typeid(other);

			if (my_type != other_type)
				return my_type.before(other_type) ? -1 : 1;

			//avoiding dynamic_cast here
			const char * this_ptr = reinterpret_cast<const char *>(this);
			const char * this_icomparable = reinterpret_cast<const char *>(static_cast<const IComparableNew *>(this));
			ptrdiff_t delta = this_ptr - this_icomparable; //distance between ComparableNew and IComparableNew for this type.
			const char * other_icomparable = reinterpret_cast<const char *>(&other);
			const ComparableNew<T> *other_ptr = reinterpret_cast<const ComparableNew<T> *>(other_icomparable + delta);

			return this->DoCompare(*static_cast<const T*>(other_ptr));
		}

	protected:
		virtual int DoCompare(const T &other) const = 0;
	};


	struct ComparableNewCmp
	{
		int operator()(const shared_ptr<IComparableNew>& l, const shared_ptr<IComparableNew>& r) const	{ return Compare(l, r); }
		int operator()(const IComparableNew& l, const IComparableNew& r) const							{ return Compare(l, r); }

		int Compare(const shared_ptr<IComparableNew>& l, const shared_ptr<IComparableNew>& r) const		{ return (l && r) ? Compare(*l, *r) : StandardOperatorsComparer()(l.get(), r.get()); }
		int Compare(const IComparableNew& l, const IComparableNew& r) const								{ return l.Compare(r); }
	};


	struct ComparableNewLess
	{
		bool operator()(const shared_ptr<IComparableNew>& l, const shared_ptr<IComparableNew>& r) const	{ return Compare(l, r); }
		bool operator()(const IComparableNew& l, const IComparableNew& r) const							{ return Compare(l, r); }

		bool Compare(const shared_ptr<IComparableNew>& l, const shared_ptr<IComparableNew>& r) const	{ return (l && r) ? Compare(*l, *r) : (l.get() < r.get()); }
		bool Compare(const IComparableNew& l, const IComparableNew& r) const							{ return l.Compare(r) < 0; }
	};


	struct ComparableNewEquals
	{
		bool operator()(const shared_ptr<IComparableNew>& l, const shared_ptr<IComparableNew>& r) const	{ return Compare(l, r); }
		bool operator()(const IComparableNew& l, const IComparableNew& r) const							{ return Compare(l, r); }

		bool Compare(const shared_ptr<IComparableNew>& l, const shared_ptr<IComparableNew>& r) const	{ return (l && r) ? Compare(*l, *r) : (l.get() == r.get()); }
		bool Compare(const IComparableNew& l, const IComparableNew& r) const							{ return l.Compare(r) == 0; }
	};


}

#endif
