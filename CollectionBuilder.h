#ifndef STINGRAY_TOOLKIT_COLLECTIONBUILDER_H
#define STINGRAY_TOOLKIT_COLLECTIONBUILDER_H


#include <set>
#include <map>
#include <vector>
#include <stingray/toolkit/NestedTypeCheck.h>


namespace stingray
{

	/**
	 * @addtogroup toolkit_collections
	 * @{
	 */


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


#define DETAIL_CREATE_BUILDER(BuilderName, ...) \
	class BuilderName : public CollectionBuilder<__VA_ARGS__> \
	{ \
		typedef CollectionBuilder<__VA_ARGS__> base; \
		typedef typename base::ArgType ArgType; \
	public: \
		BuilderName() {} \
		BuilderName(ArgType a1) : base(a1) { } \
		BuilderName(ArgType a1, ArgType a2) : base(a1, a2) { } \
		BuilderName(ArgType a1, ArgType a2, ArgType a3) : base(a1, a2, a3) { } \
		BuilderName(ArgType a1, ArgType a2, ArgType a3, ArgType a4) : base(a1, a2, a3, a4) { } \
		BuilderName(ArgType a1, ArgType a2, ArgType a3, ArgType a4, ArgType a5) : base(a1, a2, a3, a4, a5) { } \
		BuilderName(ArgType a1, ArgType a2, ArgType a3, ArgType a4, ArgType a5, ArgType a6) : base(a1, a2, a3, a4, a5, a6) { } \
		BuilderName(ArgType a1, ArgType a2, ArgType a3, ArgType a4, ArgType a5, ArgType a6, ArgType a7) : base(a1, a2, a3, a4, a5, a6, a7) { } \
		BuilderName(ArgType a1, ArgType a2, ArgType a3, ArgType a4, ArgType a5, ArgType a6, ArgType a7, ArgType a8) : base(a1, a2, a3, a4, a5, a6, a7, a8) { } \
		BuilderName(ArgType a1, ArgType a2, ArgType a3, ArgType a4, ArgType a5, ArgType a6, ArgType a7, ArgType a8, ArgType a9) : base(a1, a2, a3, a4, a5, a6, a7, a8, a9) { } \
		BuilderName(ArgType a1, ArgType a2, ArgType a3, ArgType a4, ArgType a5, ArgType a6, ArgType a7, ArgType a8, ArgType a9, ArgType a10) : base(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10) { } \
	}

	template < typename T >
	DETAIL_CREATE_BUILDER(VectorBuilder, std::vector<T>);

	template < typename T >
	DETAIL_CREATE_BUILDER(SetBuilder, std::set<T>);

	template < typename K, typename V >
	DETAIL_CREATE_BUILDER(MapBuilder, std::map<K, V>);
#undef DETAIL_CREATE_BUILDER


	/** @} */

}


#endif
