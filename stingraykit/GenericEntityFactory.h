#ifndef STINGRAYKIT_GENERICENTITYFACTORY_H
#define STINGRAYKIT_GENERICENTITYFACTORY_H

// Copyright (c) 2011 - 2015, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/exception.h>
#include <stingraykit/TypeList.h>

namespace stingray
{

	namespace Detail
	{

		template < typename Registry, typename TagType, typename ReturnType, template <typename> class LeafFunc >
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
				{ return LeafFunc<ReturnType>::Do(TagType(tag)); }
				STINGRAYKIT_REPEAT_NESTING_2(5, DETAIL_STINGRAYKIT_DECLARE_LEAFNODE_PROCESS, ~)
#undef DETAIL_STINGRAYKIT_DECLARE_LEAFNODE_PROCESS
			};

		public:
			typedef typename BalancedTypeTree<Registry, RegistryEntryLess, BranchNode, LeafNode>::ValueT ValueT;
		};

	}


	class UnknownEntityTagException : public Exception
	{
	public:
		template < typename EntityTagType >
		UnknownEntityTagException(EntityTagType tag) :
			Exception("Unknown tag: " + tag.ToString())
		{ }
	};


	template < typename ReturnType >
	struct ThrowLeafFunc
	{
		template < typename TagType >
		static ReturnType Do(TagType tag) { throw UnknownEntityTagException(tag); }
	};


	template < typename ReturnType >
	struct DefaultConstructorLeafFunc
	{
		template < typename TagType >
		static ReturnType Do(TagType tag) { return ReturnType(); }
	};


	template < typename ReturnType >
	struct NullPtrLeafFunc
	{
		template < typename TagType >
		static ReturnType Do(TagType tag) { return null; }
	};


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
			typedef typename Detail::EnumDrivenInvoker<typename Derived::Registry, TagType, ReturnType, DefaultConstructorLeafFunc>::ValueT Invoker; \
			return Invoker::Process(tag STINGRAYKIT_COMMA_IF(N_) STINGRAYKIT_REPEAT(N_, STINGRAYKIT_FUNCTION_PARAM_USAGE, T)); \
		}
		STINGRAYKIT_REPEAT_NESTING_2(5, DETAIL_STINGRAYKIT_DECLARE_GENERICINVOKER_INVOKE, ~)
#undef DETAIL_STINGRAYKIT_DECLARE_GENERICINVOKER_INVOKE
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
			typedef typename Detail::EnumDrivenInvoker<typename TypeListTransform<typename Derived::Registry, ToEntityCreator>::ValueT, EntityTagType, EntityPtr, ThrowLeafFunc>::ValueT Registry;

			EntityTagType tag;
			{
				typename StreamType::Rollback rollback(stream);
				tag = EntityTagReader::Read(stream);
			}

			return Registry::Process(tag, stream);
		}
	};

}

#endif
