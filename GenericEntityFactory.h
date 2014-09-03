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

			template <typename Left, typename Right>
			struct RegistryEntryLess
			{ static const bool Value = Left::Tag < Right::Tag; };

			template < typename EntityType >
			struct EntityCreator
			{
				template < typename StreamType >
				static EntityPtr Create(StreamType& stream)
				{ return make_shared<EntityType>(); }
			};

			struct EmptyNode
			{
				template < typename StreamType >
				static EntityPtr Create(typename EntityTagType::Enum tag, StreamType& stream)
				{ return null; }
			};

			template < typename TypeList, size_t Begin = 0, size_t End = GetTypeListLength<TypeList>::Value >
			struct Node
			{
			private:
				static const size_t Index = Begin + (End - Begin) / 2;

				typedef typename GetTypeListItem<TypeList, Index>::ValueT	Item;

			public:
				template < typename StreamType >
				static EntityPtr Create(typename EntityTagType::Enum tag, StreamType& stream)
				{
					if (tag == Item::Tag)
						return If<Inherits<typename Item::Type, BaseEntityType>::Value, EntityCreator<typename Item::Type>, typename Item::Type>::ValueT::Create(stream);
					else
						return tag < Item::Tag ? Node<TypeList, Begin, Index>::Create(tag, stream) : Node<TypeList, Index + 1, End>::Create(tag, stream);
				}
			};

			template < typename TypeList, size_t Begin >
			struct Node<TypeList, Begin, Begin>
			{
				template < typename StreamType >
				static EntityPtr Create(typename EntityTagType::Enum tag, StreamType& stream)
				{ return null; }
			};

			template < typename TypeList >
			struct Node<TypeList, 0, 0>
			{
				template < typename StreamType >
				static EntityPtr Create(typename EntityTagType::Enum tag, StreamType& stream)
				{ return null; }
			};

			typedef Node<typename TypeListSort<Registry, RegistryEntryLess>::ValueT> Root;

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

}


#endif
