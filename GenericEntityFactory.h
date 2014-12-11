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
#define DETAIL_TOOLKIT_DECLARE_BRANCHNODE_PROCESS(N_, UserArg_) \
			TOOLKIT_INSERT_IF(N_, template <) TOOLKIT_REPEAT(N_, TOOLKIT_TEMPLATE_PARAM_DECL, T) TOOLKIT_INSERT_IF(N_, >) \
			static ReturnType Process(typename TagType::Enum tag TOOLKIT_COMMA_IF(N_) TOOLKIT_REPEAT(N_, TOOLKIT_FUNCTION_PARAM_DECL_BYVALUE, T)) \
			{ \
				if (tag == Entry::Tag) \
					return Entry::Type::Do(TOOLKIT_REPEAT(N_, TOOLKIT_FUNCTION_PARAM_USAGE, T)); \
				else \
					return tag < Entry::Tag ? LeftNode::Process(tag TOOLKIT_COMMA_IF(N_) TOOLKIT_REPEAT(N_, TOOLKIT_FUNCTION_PARAM_USAGE, T)) : RightNode::Process(tag TOOLKIT_COMMA_IF(N_) TOOLKIT_REPEAT(N_, TOOLKIT_FUNCTION_PARAM_USAGE, T)); \
			}
			TOOLKIT_REPEAT_NESTING_2(5, DETAIL_TOOLKIT_DECLARE_BRANCHNODE_PROCESS, ~)
#undef DETAIL_TOOLKIT_DECLARE_BRANCHNODE_PROCESS
		};

		struct LeafNode
		{
#define DETAIL_TOOLKIT_DECLARE_LEAFNODE_PROCESS(N_, UserArg_) \
			TOOLKIT_INSERT_IF(N_, template <) TOOLKIT_REPEAT(N_, TOOLKIT_TEMPLATE_PARAM_DECL, T) TOOLKIT_INSERT_IF(N_, >) \
			static ReturnType Process(typename TagType::Enum tag TOOLKIT_COMMA_IF(N_) TOOLKIT_REPEAT(N_, TOOLKIT_FUNCTION_PARAM_DECL_BYVALUE, T)) \
			{ return ReturnType(); }
			TOOLKIT_REPEAT_NESTING_2(5, DETAIL_TOOLKIT_DECLARE_LEAFNODE_PROCESS, ~)
#undef DETAIL_TOOLKIT_DECLARE_LEAFNODE_PROCESS
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

	template < typename BaseEntityType, typename StreamType_ >
	struct IEntityFactory
	{
		typedef StreamType_									StreamType;
		typedef IEntityFactory<BaseEntityType, StreamType>	InterfaceType;
		typedef shared_ptr<BaseEntityType>					EntityPtr;

		virtual ~IEntityFactory() { }

		virtual EntityPtr Create(StreamType& stream) const = 0;
	};

	template < typename InterfaceType, typename EntityFactory >
	struct EntityFactoryWrapper : public InterfaceType
	{
		typedef typename InterfaceType::StreamType		StreamType;
		typedef typename InterfaceType::EntityPtr		EntityPtr;

		virtual EntityPtr Create(StreamType& stream) const
		{ return EntityFactory::Create(stream); }
	};

}


#endif
