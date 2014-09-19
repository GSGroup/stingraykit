#ifndef STINGRAY_TOOLKIT_GENERICENTITYFACTORY_H
#define STINGRAY_TOOLKIT_GENERICENTITYFACTORY_H


#include <stingray/toolkit/exception.h>
#include <stingray/toolkit/TypeList.h>


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

	template < typename EntityTagType, size_t EntityTagOffset, size_t EntityTagLength >
	struct GenericEntityTagReader
	{
		template < typename StreamType >
		static EntityTagType Read(StreamType& stream)
		{
			stream.Skip(EntityTagOffset);
			return stream.template Read<EntityTagLength>();
		}
	};

	template < typename Derived, typename BaseEntityType, typename EntityTagType, typename EntityTagReader = GenericEntityTagReader<EntityTagType, 0, 8> >
	class GenericEntityFactory
	{
		typedef shared_ptr<BaseEntityType>		EntityPtr;

		template < typename Registry >
		class EntityCreatorRegistry
		{

			template < typename Left, typename Right >
			struct RegistryEntryLess
			{ static const bool Value = Left::Tag < Right::Tag; };

			struct NullCreatorNode
			{
				template < typename StreamType >
				static EntityPtr Create(typename EntityTagType::Enum tag, StreamType& stream)
				{ return null; }
			};

			template < typename Entry, typename Left, typename Right >
			struct EntityCreatorNode
			{

				template < typename EntityType >
				struct DefaultEntityCreator
				{
					template < typename StreamType >
					static EntityPtr Create(StreamType& stream)
					{ return make_shared<EntityType>(); }
				};

				template < typename StreamType >
				static EntityPtr Create(typename EntityTagType::Enum tag, StreamType& stream)
				{
					if (tag == Entry::Tag)
						return If<Inherits<typename Entry::Type, BaseEntityType>::Value, DefaultEntityCreator<typename Entry::Type>, typename Entry::Type>::ValueT::Create(stream);
					else
						return tag < Entry::Tag ? Left::Create(tag, stream) : Right::Create(tag, stream);
				}
			};

			typedef typename BalancedTypeTree<Registry, RegistryEntryLess, EntityCreatorNode, NullCreatorNode>::ValueT Root;

		public:
			template < typename StreamType >
			static EntityPtr Create(typename EntityTagType::Enum tag, StreamType& stream)
			{ return Root::Create(tag, stream); }
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

	template < typename BaseEntityType, typename StreamType >
	struct IEntityFactory
	{
		typedef shared_ptr<BaseEntityType>		EntityPtr;

		virtual ~IEntityFactory() { }

		virtual EntityPtr Create(StreamType& stream) const = 0;

		template < typename EntityFactory >
		struct Wrapper : public IEntityFactory<BaseEntityType, StreamType>
		{
			virtual EntityPtr Create(StreamType& stream) const { return EntityFactory::Create(stream); }
		};
	};

}


#endif
