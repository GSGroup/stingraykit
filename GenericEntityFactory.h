#ifndef STINGRAY_TOOLKIT_GENERICENTITYFACTORY_H
#define STINGRAY_TOOLKIT_GENERICENTITYFACTORY_H


#include <stingray/toolkit/exception.h>


namespace stingray
{

	class UnknownEntityTagException : public Exception
	{
	public:
		template < typename EntityTagType >
		UnknownEntityTagException(EntityTagType tag)
			: Exception("Unknown tag: " + tag.ToString())
		{ }
	};

	template < typename EntityTagType, size_t EntityTagLength >
	struct GenericEntityTagReader
	{
		template < typename StreamType >
		static EntityTagType Read(StreamType& stream)
		{ return stream.template Read<EntityTagLength>(); }
	};

	template < typename Derived, typename BaseEntityType, typename EntityTagType, typename EntityTagReader = GenericEntityTagReader<EntityTagType, 8> >
	class GenericEntityFactory
	{
		typedef shared_ptr<BaseEntityType>		EntityPtr;

		template < typename Registry >
		class EntityCreatorRegistry
		{
			template < typename EntityType >
			struct EntityCreator
			{
				template < typename StreamType >
				static EntityPtr Create(StreamType& stream)
				{ return make_shared<EntityType>(); }
			};

			struct LastEntry
			{
				template < typename StreamType >
				static EntityPtr Create(typename EntityTagType::Enum tag, StreamType& stream)
				{ return null; }
			};

			template < typename EntityTagType::Enum Tag, typename Type, typename NextEntry >
			struct Entry
			{
				template < typename StreamType >
				static EntityPtr Create(typename EntityTagType::Enum tag, StreamType& stream)
				{ return tag == Tag ? Type::Create(stream) : NextEntry::Create(tag, stream); }
			};

			template < typename RegistryEntry, typename Result >
			struct EntryAccumulator
			{
				typedef Entry<RegistryEntry::Tag, typename If<Inherits<typename RegistryEntry::Type, BaseEntityType>::Value, EntityCreator<typename RegistryEntry::Type>, typename RegistryEntry::Type>::ValueT, Result>	ValueT;
			};

			typedef typename TypeListAccumulate<typename ToTypeList<Registry>::ValueT, EntryAccumulator, LastEntry>::ValueT		Entries;

		public:
			template < typename StreamType >
			static EntityPtr Create(typename EntityTagType::Enum tag, StreamType& stream)
			{ return Entries::Create(tag, stream); }
		};

	public:
		template < typename EntityTagType::Enum EntityTag, typename TargetType >
		struct RegistryEntry
		{
			static const typename EntityTagType::Enum Tag = EntityTag;
			typedef TargetType Type;
		};

		template < typename StreamType >
		static EntityPtr Create(StreamType& stream)
		{
			typedef EntityCreatorRegistry<typename ToTypeList<typename Derived::Registry>::ValueT>	Registry;

			EntityTagType tag;
			{
				typename StreamType::Rollback rollback(stream);
				tag = EntityTagReader::Read(stream);
			}

			EntityPtr result = Registry::Create(tag, stream);
			TOOLKIT_CHECK(result, UnknownEntityTagException(tag));

			return result;
		}
	};

}


#endif
