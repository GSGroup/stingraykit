#ifndef STINGRAY_TOOLKIT_STLENUMERATORADAPTER_H
#define STINGRAY_TOOLKIT_STLENUMERATORADAPTER_H


#include <stingray/toolkit/IEnumerator.h>
#include <stingray/toolkit/shared_ptr.h>


/*! \cond GS_INTERNAL */

namespace stingray
{

	
	template < typename T >
	class StlEnumeratorAdapter : public std::iterator< std::forward_iterator_tag, T >
	{
	private:
		shared_ptr<IEnumerator<T> >		_enumerator;

	public:
		StlEnumeratorAdapter() // This means the end of a collection
		{ }

		StlEnumeratorAdapter(shared_ptr<IEnumerator<T> > enumerator)
			: _enumerator(enumerator)
		{ }

		const StlEnumeratorAdapter& operator ++ ()
		{
			if (_enumerator->Valid()) // Should we throw if _enumerator is not valid?
				_enumerator->Next();
			return *this;
		}

		T operator * () const
		{
			if (!_enumerator->Valid())
				TOOLKIT_THROW(std::runtime_error("Trying to dereference an invalid enumerator!"));
			return _enumerator->Get();
		}

		bool operator != (const StlEnumeratorAdapter& other)
		{
			if (other._enumerator != NULL)
				TOOLKIT_THROW(NotImplementedException());
			return _enumerator->Valid();
		}

		// whatever
	};


	template < typename T >
	FORCE_INLINE StlEnumeratorAdapter<T> Wrap(shared_ptr<IEnumerator<T> > enumerator)
	{ return StlEnumeratorAdapter<T>(enumerator); }

	template < typename T >
	FORCE_INLINE StlEnumeratorAdapter<T> WrapEnd(shared_ptr<IEnumerator<T> >/* enumerator*/)
	{ return StlEnumeratorAdapter<T>(); }


}

/*! \endcond */


#endif
