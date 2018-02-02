#ifndef STINGRAYKIT_COLLECTION_REFCOUNTER_H
#define STINGRAYKIT_COLLECTION_REFCOUNTER_H

#include <stddef.h>

namespace stingray
{

	class RefCounter
	{
	private:
		size_t		_refs;

	public:
		RefCounter() : _refs() { }

		size_t count() const
		{ return _refs; }

		template < typename DoSetFunc >
		void set(const DoSetFunc& doSetFunc)
		{
			if (_refs++ == 0)
				doSetFunc();
		}

		template < typename DoReleaseFunc >
		bool release(const DoReleaseFunc& doReleaseFunc)
		{
			if (_refs == 0)
				return false;

			if (--_refs == 0)
				doReleaseFunc();

			return true;
		}

		template < typename DoReleaseFunc >
		void reset(const DoReleaseFunc& doReleaseFunc)
		{
			if (_refs != 0)
				doReleaseFunc();

			_refs = 0;
		}
	};

}

#endif
