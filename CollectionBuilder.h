#ifndef STINGRAY_TOOLKIT_COLLECTIONBUILDER_H
#define STINGRAY_TOOLKIT_COLLECTIONBUILDER_H


#include <set>
#include <map>
#include <vector>
#include <stingray/toolkit/NestedTypeCheck.h>


namespace stingray
{


	namespace Detail
	{
		TOOLKIT_DECLARE_METHOD_CHECK(push_back);

		template < typename CollectionType, bool HasPushBack = HasMethod_push_back<CollectionType>::Value>
		struct CollectionInserter
		{
			static void Insert(CollectionType& collection, typename GetConstReferenceType<typename CollectionType::value_type>::ValueT val)
			{ collection.push_back(val); }
		};

		template < typename CollectionType >
		struct CollectionInserter<CollectionType, false>
		{
			static void Insert(CollectionType& collection, typename GetConstReferenceType<typename CollectionType::value_type>::ValueT val)
			{ collection.insert(val); }
		};
	}


	template < typename CollectionType >
	class CollectionBuilder
	{
	public:
		typedef typename GetConstReferenceType<typename CollectionType::value_type>::ValueT ArgType;

	private:
		CollectionType		_collection;

	public:
		CollectionBuilder()
		{}
		CollectionBuilder(ArgType a1)
		{ *this % a1; }
		CollectionBuilder(ArgType a1, ArgType a2)
		{ *this % a1 % a2; }
		CollectionBuilder(ArgType a1, ArgType a2, ArgType a3)
		{ *this % a1 % a2 % a3; }
		CollectionBuilder(ArgType a1, ArgType a2, ArgType a3, ArgType a4)
		{ *this % a1 % a2 % a3 % a4; }
		CollectionBuilder(ArgType a1, ArgType a2, ArgType a3, ArgType a4, ArgType a5)
		{ *this % a1 % a2 % a3 % a4 % a5; }
		CollectionBuilder(ArgType a1, ArgType a2, ArgType a3, ArgType a4, ArgType a5, ArgType a6)
		{ *this % a1 % a2 % a3 % a4 % a5 % a6; }
		CollectionBuilder(ArgType a1, ArgType a2, ArgType a3, ArgType a4, ArgType a5, ArgType a6, ArgType a7)
		{ *this % a1 % a2 % a3 % a4 % a5 % a6 % a7; }
		CollectionBuilder(ArgType a1, ArgType a2, ArgType a3, ArgType a4, ArgType a5, ArgType a6, ArgType a7, ArgType a8)
		{ *this % a1 % a2 % a3 % a4 % a5 % a6 % a7 % a8; }
		CollectionBuilder(ArgType a1, ArgType a2, ArgType a3, ArgType a4, ArgType a5, ArgType a6, ArgType a7, ArgType a8, ArgType a9)
		{ *this % a1 % a2 % a3 % a4 % a5 % a6 % a7 % a8 % a9; }
		CollectionBuilder(ArgType a1, ArgType a2, ArgType a3, ArgType a4, ArgType a5, ArgType a6, ArgType a7, ArgType a8, ArgType a9, ArgType a10)
		{ *this % a1 % a2 % a3 % a4 % a5 % a6 % a7 % a8 % a9 % a10; }

		const CollectionType& Get() const { return _collection; }

		operator const CollectionType& () const { return Get(); }

		CollectionBuilder& operator % (ArgType val)
		{ Detail::CollectionInserter<CollectionType>::Insert(_collection, val); return *this; }
	};


	template < typename T >
	class VectorBuilder : public CollectionBuilder<std::vector<T> > { };

	template < typename T >
	class SetBuilder : public CollectionBuilder<std::set<T> > { };

	template < typename K, typename V >
	class MapBuilder : public CollectionBuilder<std::map<K, V> > { };


}


#endif
