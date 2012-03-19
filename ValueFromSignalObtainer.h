#ifndef __GS_DVRLIB_TOOLKIT_VALUEFROMSIGNALOBTAINER_H__
#define __GS_DVRLIB_TOOLKIT_VALUEFROMSIGNALOBTAINER_H__


#include <dvrlib/toolkit/shared_ptr.h>
#include <dvrlib/toolkit/unique_ptr.h>


namespace dvrlib
{


	template < typename T >
	class ValueFromSignalObtainer
	{
		typedef unique_ptr<T>		TPtr;
		typedef shared_ptr<TPtr>	TPtrPtr;

	private:
		mutable TPtrPtr	_val;

	public:
		ValueFromSignalObtainer() : _val(new TPtr) { }
		
		void operator() (const T& val) const
		{
			TOOLKIT_CHECK(!*_val, "Value already set!");
			_val->reset(new T(val));
		}

		const T& GetValue() const
		{
			TOOLKIT_CHECK(*_val, "Value is not set!");
			return **_val;
		}

		bool HasValue() const { return *_val; }
	};

}


#endif
