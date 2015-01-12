#ifndef STINGRAY_TOOLKIT_GENERICENTITYFACTORY_H
#define STINGRAY_TOOLKIT_GENERICENTITYFACTORY_H


#include <stingray/toolkit/exception.h>
#include <stingray/toolkit/TypeList.h>


namespace stingray
{

	namespace Detail
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
#define DETAIL_STINGRAYKIT_DECLARE_BRANCHNODE_PROCESS(N_, UserArg_) \
				STINGRAYKIT_INSERT_IF(N_, template <) STINGRAYKIT_REPEAT(N_, STINGRAYKIT_TEMPLATE_PARAM_DECL, T) STINGRAYKIT_INSERT_IF(N_, >) \
				static ReturnType Process(typename TagType::Enum tag STINGRAYKIT_COMMA_IF(N_) STINGRAYKIT_REPEAT(N_, STINGRAYKIT_FUNCTION_PARAM_DECL_BYVALUE, T)) \
				{ \
					if (tag == Entry::Tag) \
						return Entry::Type::Do(STINGRAYKIT_REPEAT(N_, STINGRAYKIT_FUNCTION_PARAM_USAGE, T)); \
					else \
						return tag < Entry::Tag ? LeftNode::Process(tag STINGRAYKIT_COMMA_IF(N_) STINGRAYKIT_REPEAT(N_, STINGRAYKIT_FUNCTION_PARAM_USAGE, T)) : RightNode::Process(tag STINGRAYKIT_COMMA_IF(N_) STINGRAYKIT_REPEAT(N_, STINGRAYKIT_FUNCTION_PARAM_USAGE, T)); \
				}
				STINGRAYKIT_REPEAT_NESTING_2(5, DETAIL_STINGRAYKIT_DECLARE_BRANCHNODE_PROCESS, ~)
#undef DETAIL_STINGRAYKIT_DECLARE_BRANCHNODE_PROCESS
			};

			struct LeafNode
			{
#define DETAIL_STINGRAYKIT_DECLARE_LEAFNODE_PROCESS(N_, UserArg_) \
				STINGRAYKIT_INSERT_IF(N_, template <) STINGRAYKIT_REPEAT(N_, STINGRAYKIT_TEMPLATE_PARAM_DECL, T) STINGRAYKIT_INSERT_IF(N_, >) \
				static ReturnType Process(typename TagType::Enum tag STINGRAYKIT_COMMA_IF(N_) STINGRAYKIT_REPEAT(N_, STINGRAYKIT_FUNCTION_PARAM_DECL_BYVALUE, T)) \
				{ return ReturnType(); }
				STINGRAYKIT_REPEAT_NESTING_2(5, DETAIL_STINGRAYKIT_DECLARE_LEAFNODE_PROCESS, ~)
#undef DETAIL_STINGRAYKIT_DECLARE_LEAFNODE_PROCESS
			};

		public:
			typedef typename BalancedTypeTree<Registry, RegistryEntryLess, BranchNode, LeafNode>::ValueT ValueT;
		};

	}

	template < typename Derived, typename TagType, typename ReturnType >
	class GenericInvoker
	{
	protected:
		template < typename TagType::Enum Tag_, typename TargetType >
		struct RegistryEntry
		{
			static const typename TagType::Enum Tag = Tag_;
			typedef TargetType Type;
		};

	public:
#define DETAIL_STINGRAYKIT_DECLARE_GENERICINVOKER_INVOKE(N_, UserArg_) \
		STINGRAYKIT_INSERT_IF(N_, template <) STINGRAYKIT_REPEAT(N_, STINGRAYKIT_TEMPLATE_PARAM_DECL, T) STINGRAYKIT_INSERT_IF(N_, >) \
		static ReturnType Invoke(typename TagType::Enum tag STINGRAYKIT_COMMA_IF(N_) STINGRAYKIT_REPEAT(N_, STINGRAYKIT_FUNCTION_PARAM_DECL_BYVALUE, T)) \
		{ \
			typedef typename Detail::EnumDrivenInvoker<typename Derived::Registry, TagType, ReturnType>::ValueT Invoker; \
			return Invoker::Process(tag STINGRAYKIT_COMMA_IF(N_) STINGRAYKIT_REPEAT(N_, STINGRAYKIT_FUNCTION_PARAM_USAGE, T)); \
		}
		STINGRAYKIT_REPEAT_NESTING_2(5, DETAIL_STINGRAYKIT_DECLARE_GENERICINVOKER_INVOKE, ~)
#undef DETAIL_STINGRAYKIT_DECLARE_GENERICINVOKER_INVOKE
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
			typedef typename Detail::EnumDrivenInvoker<typename TypeListTransform<typename Derived::Registry, ToEntityCreator>::ValueT, EntityTagType, EntityPtr>::ValueT Registry;

			EntityTagType tag;
			{
				typename StreamType::Rollback rollback(stream);
				tag = EntityTagReader::Read(stream);
			}

			EntityPtr result = Registry::Process(tag, stream);
			STINGRAYKIT_CHECK(result, UnknownEntityTagException(tag));

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
