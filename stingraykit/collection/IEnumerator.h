#ifndef STINGRAYKIT_COLLECTION_IENUMERATOR_H
#define STINGRAYKIT_COLLECTION_IENUMERATOR_H

#include <stingraykit/function/function.h>
#include <stingraykit/shared_ptr.h>

#define STINGRAYKIT_DECLARE_ENUMERATOR(ClassName) \
		typedef stingray::IEnumerator<ClassName>				ClassName##Enumerator; \
		STINGRAYKIT_DECLARE_PTR(ClassName##Enumerator)

namespace stingray
{

	/**
	 * @addtogroup toolkit_collections
	 * @{
	 */

	template < typename T >
	struct IEnumerator
	{
		typedef T		ItemType;

		virtual ~IEnumerator() { }

		virtual bool Valid() const = 0;
		virtual T Get() const = 0;
		virtual void Next() = 0;
	};


	template < typename T >
	struct IsEnumerator
	{
	private:
		template < typename U >
		static YesType GetIsEnumerator(const IEnumerator<U>*);
		static NoType GetIsEnumerator(...);

	public:
		static const bool Value = sizeof(GetIsEnumerator((const T*)0)) == sizeof(YesType);
	};


	/** @} */

}


#endif
