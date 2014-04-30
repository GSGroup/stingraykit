#ifndef STINGRAY_TOOLKIT_GENERICENTITYFACTORY_H
#define STINGRAY_TOOLKIT_GENERICENTITYFACTORY_H


#include <map>

#include <stingray/log/Logger.h>
#include <stingray/toolkit/ICreator.h>


namespace stingray
{

	class UnknowEntityTagException : public Exception
	{
	public:
		template < typename EntityTagType >
		UnknowEntityTagException(EntityTagType tag)
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
		typedef shared_ptr<BaseEntityType> 					EntityPtr;
		typedef ICreator<BaseEntityType>					EntityCreator;
		typedef shared_ptr<EntityCreator>					EntityCreatorPtr;
		typedef std::map<EntityTagType, EntityCreatorPtr>	EntityCreatorRegistry;

		template < typename _RegisterEntry >
		struct EntityRegistrator
		{
			static void Call(GenericEntityFactory* factory)
			{ factory->Register<_RegisterEntry::Tag, typename _RegisterEntry::Type>(); }
		};

	private:
		EntityCreatorRegistry	_registry;

	public:
		template < typename EntityTagType::Enum EntityTag, typename EntityType >
		struct RegistryEntry
		{
			static const typename EntityTagType::Enum Tag = EntityTag;
			typedef EntityType Type;
		};

		GenericEntityFactory()
		{
			typedef typename Derived::Registry EntityRegistry;

			ForEachInTypeList<EntityRegistry, EntityRegistrator>::Do(this);
		}

		template < typename StreamType >
		EntityPtr Create(StreamType& stream) const
		{
			typename StreamType::Rollback rollback(stream);

			EntityTagType tag = EntityTagReader::Read(stream);
			typename EntityCreatorRegistry::const_iterator it = _registry.find(tag);
			TOOLKIT_CHECK(it != _registry.end(), UnknowEntityTagException(tag));

			return it->second->Create();
		}

	private:
		template < typename EntityTagType::Enum EntityTag, typename EntityType >
		void Register()
		{
			_registry.insert(std::make_pair(EntityTag, make_shared<DefaultConstructorCreator<BaseEntityType, EntityType> >()));
		}
	};

}


#endif
