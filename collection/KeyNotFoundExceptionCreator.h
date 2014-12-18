#ifndef STINGRAY_TOOLKIT_COLLECTION_KEYNOTFOUNDEXCEPTIONCREATOR_H
#define STINGRAY_TOOLKIT_COLLECTION_KEYNOTFOUNDEXCEPTIONCREATOR_H


#include <stingray/toolkit/StringUtils.h>
#include <stingray/toolkit/exception.h>


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
