#ifndef __GS_DVRLIB_TOOLKIT_ICOMPARER_H__
#define __GS_DVRLIB_TOOLKIT_ICOMPARER_H__


#include <dvrlib/toolkit/shared_ptr.h>


#define TOOLKIT_DECLARE_COMPARER(ClassName) \
		typedef dvrlib::IComparer<ClassName>				ClassName##Comparer; \
		TOOLKIT_DECLARE_PTR(ClassName##Comparer);

namespace dvrlib
{


	template < typename T >
	struct IComparer
	{
		virtual ~IComparer() { }

		virtual int Compare(const shared_ptr<T>& left, const shared_ptr<T>& right) const = 0;

		virtual bool Less(const shared_ptr<T>& left, const shared_ptr<T>& right) const { return Compare(left, right) < 0; }
		virtual bool Greater(const shared_ptr<T>& left, const shared_ptr<T>& right) const { return Compare(left, right) > 0; }
		virtual bool Equal(const shared_ptr<T>& left, const shared_ptr<T>& right) const { return Compare(left, right) == 0; }
		virtual bool LessOrEqual(const shared_ptr<T>& left, const shared_ptr<T>& right) const { return Compare(left, right) <= 0; }
		virtual bool GreaterOrEqual(const shared_ptr<T>& left, const shared_ptr<T>& right) const { return Compare(left, right) >= 0; }
	};


	template < typename T >
	class InverseComparer : public virtual IComparer<T>
	{
	private:
		shared_ptr<IComparer<T> >	_originalComparer;

	public:
		FORCE_INLINE InverseComparer(shared_ptr<IComparer<T> > originalComparer) 
			: _originalComparer(originalComparer)
		{ }

		virtual int Compare(const shared_ptr<T>& left, const shared_ptr<T>& right) const
		{ return -_originalComparer->Compare(left, right); }
	};


	template < typename T >
	FORCE_INLINE shared_ptr<IComparer<T> > Invert(const shared_ptr<IComparer<T> >& comparer)
	{ return make_shared<InverseComparer<T> >(comparer); }

}


#endif
