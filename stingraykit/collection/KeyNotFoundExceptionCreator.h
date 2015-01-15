#ifndef STINGRAYKIT_COLLECTION_KEYNOTFOUNDEXCEPTIONCREATOR_H
#define STINGRAYKIT_COLLECTION_KEYNOTFOUNDEXCEPTIONCREATOR_H


#include <stingraykit/string/StringUtils.h>
#include <stingraykit/exception.h>


namespace stingray
{

	/**
	 * @addtogroup toolkit_collections
	 * @{
	 */

	namespace Detail
	{
		template < typename KeyType, bool StringRepresentable = IsStringRepresentable<KeyType>::Value >
		struct KeyNotFoundExceptionCreator
		{ static KeyNotFoundException Create(const KeyType& key) { return KeyNotFoundException(ToString(key)); } };

		template < typename KeyType >
		struct KeyNotFoundExceptionCreator<KeyType, false>
		{ static KeyNotFoundException Create(const KeyType& key) { return KeyNotFoundException(); } };
	}

	template < typename KeyType >
	KeyNotFoundException CreateKeyNotFoundException(const KeyType& key)
	{ return Detail::KeyNotFoundExceptionCreator<KeyType>::Create(key); }

	/** @} */

}


#endif
