#ifndef STINGRAY_TOOLKIT_GENERICENTITYFACTORY_H
#define STINGRAY_TOOLKIT_GENERICENTITYFACTORY_H


#include <stingray/toolkit/exception.h>
#include <stingray/toolkit/TypeList.h>


namespace stingray
{

	template < typename Registry, typename TagType, typename ReturnType >
	class EnumDrivenInvoker
	{
		template < typename Left, typename Right >
		struct RegistryEntryLess
		{ static const bool Value = Left::Tag < Right::Tag; };

		template < typename Entry, typename LeftNode, typename RightNode >
		struct BranchNode
		{
			template < typename T1 >
			static ReturnType Process(typename TagType::Enum tag, T1 p1)
			{
				if (tag == Entry::Tag)
					return Entry::Type::Do(p1);
				else
					return tag < Entry::Tag ? LeftNode::Process(tag, p1) : RightNode::Process(tag, p1);
			}
		};

		struct LeafNode
		{
			template < typename T1 >
			static ReturnType Process(typename TagType::Enum tag, T1 p1)
			{ return ReturnType(); }
		};

	public:
		typedef typename BalancedTypeTree<Registry, RegistryEntryLess, BranchNode, LeafNode>::ValueT ValueT;
	};

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
	protected:
		template < typename EntityTagType::Enum EntityTag, typename TargetType >
		struct RegistryEntry
		{
			static const typename EntityTagType::Enum Tag = EntityTag;
			typedef TargetType Type;
		};

	private:
		typedef shared_ptr<BaseEntityType>		EntityPtr;

		template < typename Entry >
		struct ToEntityCreator
		{

			template < typename EntityType >
			struct DefaultEntityCreator
			{
				template < typename StreamType >
				static EntityPtr Do(StreamType& stream)
				{ return make_shared<EntityType>(); }
			};

			template < typename FactoryType >
			struct FactoryEntityCreator
			{
				template < typename StreamType >
				static EntityPtr Do(StreamType& stream)
				{ return FactoryType::Create(stream); }
			};

			typedef RegistryEntry<Entry::Tag, typename If<Inherits<typename Entry::Type, BaseEntityType>::Value,
					DefaultEntityCreator<typename Entry::Type>,
					FactoryEntityCreator<typename Entry::Type>
			>::ValueT> ValueT;

		};

	public:
		template < typename StreamType >
		static EntityPtr Create(StreamType& stream)
		{
			typedef typename EnumDrivenInvoker<typename TypeListTransform<typename Derived::Registry, ToEntityCreator>::ValueT, EntityTagType, EntityPtr>::ValueT Registry;

			EntityTagType tag;
			{
				typename StreamType::Rollback rollback(stream);
				tag = EntityTagReader::Read(stream);
			}

			EntityPtr result = Registry::Process(tag, stream);
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
