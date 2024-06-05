#ifndef STINGRAYKIT_TYPEERASURE_H
#define STINGRAYKIT_TYPEERASURE_H

// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/core/NullPtrType.h>
#include <stingraykit/function/function_info.h>
#include <stingraykit/fatal.h>
#include <stingraykit/reference.h>

namespace stingray
{

	template < typename Concepts_, typename Wrapped_ >
	class TypeErasureImpl;


	class TypeErasureBase
	{
		template < typename C, typename W >
		friend class TypeErasureImpl;

	public:
		typedef void DetypedFunction();
		typedef DetypedFunction* DetypedFunctionPtr;
		typedef DetypedFunctionPtr VTableFunc(size_t functionIndex);

	private:
		VTableFunc*	_vTable;

	protected:
		TypeErasureBase() : _vTable(NULL)
		{ }

		~TypeErasureBase()
		{ }

	public:
		VTableFunc* GetVTable() const
		{ return _vTable; }
	};


	namespace Concepts
	{
		struct Destructor : public function_info<void ()>
		{ };
	}


	namespace Detail
	{
		template < typename TypeErasureImpl_, typename Concept_, typename... Ts >
		struct ConceptInvoker
		{
			static typename Concept_::RetType DoCall(TypeErasureBase* self, Ts... args)
			{ return Concept_::Apply(static_cast<TypeErasureImpl_&>(*self), std::forward<Ts>(args)...); }
		};


		template < typename TypeErasureImpl_ >
		struct ConceptInvoker<TypeErasureImpl_, Concepts::Destructor>
		{
			static void DoCall(TypeErasureBase* self)
			{ CheckedDelete(static_cast<TypeErasureImpl_*>(self)); }
		};


		struct TypeErasureHelper
		{
			static void Terminate()
			{ STINGRAYKIT_FATAL("Pure virtual function called"); }
		};
	}


	template < typename Concepts_, typename Base_ = TypeErasureBase >
	class TypeErasure
	{
		typedef typename TypeListPrepend<Concepts_, Concepts::Destructor>::ValueT AllConcepts;

	private:
		Base_* _data;

	public:
		TypeErasure() : _data()
		{ }

		TypeErasure(const TypeErasure& other) : _data(other._data)
		{ }

		TypeErasure(TypeErasure&& other) : _data()
		{ std::swap(_data, other._data); }

		~TypeErasure()
		{ }

		TypeErasure& operator = (const TypeErasure& other)
		{ _data = other._data; return *this; }

		TypeErasure& operator = (TypeErasure&& other)
		{ _data = null; std::swap(_data, other._data); return *this; }

		template < typename Wrapped, typename... Ts >
		Wrapped* Allocate(Ts&&... args)
		{
			Wrapped* result = new TypeErasureImpl<AllConcepts, Wrapped>(std::forward<Ts>(args)...);
			_data = result;
			return result;
		}

		void Free()
		{
			Call<Concepts::Destructor>();
			_data = NULL;
		}

		Base_* Get() const
		{ return _data; }

		template < typename Concept_, typename... Ts >
		typename Concept_::RetType Call(Ts&&... args)
		{
			static_assert(GetTypeListLength<typename Concept_::ParamTypes>::Value == sizeof...(Ts), "Params count mismatch");

			const size_t ConceptIndex = IndexOfTypeListItem<AllConcepts, Concept_>::Value;
			TypeErasureBase::DetypedFunctionPtr virtualFunc = _data->GetVTable()(ConceptIndex);

			typedef typename TypeListPrepend<typename Concept_::ParamTypes, TypeErasureBase*>::ValueT AllParams;
			typedef typename SignatureBuilder<typename Concept_::RetType, AllParams>::ValueT Signature;

			return reinterpret_cast<Signature*>(virtualFunc)(_data, std::forward<Ts>(args)...);
		}
	};


	template < typename Concepts_, typename Wrapped_ >
	class TypeErasureImpl : public Wrapped_
	{
		typedef Wrapped_								Wrapped;
		typedef Concepts_								Concepts;
		typedef TypeErasureImpl<Concepts_, Wrapped_>	Self;

	public:
		template < typename... Ts >
		TypeErasureImpl(Ts&&... args) : Wrapped(std::forward<Ts>(args)...)
		{ TypeErasureBase::_vTable = &VTableFuncImpl; }

		~TypeErasureImpl()
		{ TypeErasureBase::_vTable = NULL; }

	private:
		typedef void DetypedFunction();
		typedef DetypedFunction* DetypedFunctionPtr;

		template < size_t Index >
		struct VTableHelper
		{
			static bool Call(size_t functionIndex, DetypedFunctionPtr& result)
			{
				if (functionIndex != Index)
					return true;

				typedef typename GetTypeListItem<Concepts, Index>::ValueT Concept;

				CallImpl<Concept>(std::make_index_sequence<GetTypeListLength<typename Concept::ParamTypes>::Value>(), result);
				return false;
			}

		private:
			template < typename Concept, size_t... ParamIndex >
			static void CallImpl(std::index_sequence<ParamIndex...>, DetypedFunctionPtr& result)
			{
				typedef Detail::ConceptInvoker<Self, Concept, typename GetTypeListItem<typename Concept::ParamTypes, ParamIndex>::ValueT...> ConceptInvoker;

				result = reinterpret_cast<DetypedFunctionPtr>(&ConceptInvoker::DoCall);
			}
		};

		static DetypedFunctionPtr VTableFuncImpl(size_t functionIndex)
		{
			DetypedFunctionPtr result = NULL;
			if (ForIf<GetTypeListLength<Concepts>::Value, VTableHelper>::Do(functionIndex, wrap_ref(result)))
				Detail::TypeErasureHelper::Terminate();
			return result;
		}
	};

}

#endif
