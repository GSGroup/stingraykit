#ifndef STINGRAYKIT_TYPEERASURE_H
#define STINGRAYKIT_TYPEERASURE_H

#include <stingraykit/Macro.h>

// Copyright (c) 2011 - 2015, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/metaprogramming/ForIf.h>

namespace stingray
{

	template<typename Concepts_, typename Wrapped_>
	class TypeErasureImpl;


	class TypeErasureBase
	{
		template<typename C, typename W>
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
		struct Destructor : public function_info<void, TypeList_0>
		{ };
	}


	namespace Detail
	{
		template<typename TypeErasureImpl_, typename Concept_>
		struct ConceptInvoker
		{
			static typename Concept_::RetType DoCall(TypeErasureBase* self, const Tuple<typename Concept_::ParamTypes>& paramTuple)
			{
				typedef Tuple<typename TypeListPrepend<typename Concept_::ParamTypes, TypeErasureImpl_&>::ValueT> AllParamsTuple;
				AllParamsTuple allParams(static_cast<TypeErasureImpl_&>(*self), paramTuple);
				return FunctorInvoker::Invoke(&Concept_::template Apply<TypeErasureImpl_>, allParams);
			}
		};


		template<typename TypeErasureImpl_>
		struct ConceptInvoker<TypeErasureImpl_, Concepts::Destructor>
		{
			static void DoCall(TypeErasureBase* self, const Tuple<TypeList_0>& paramTuple)
			{ delete static_cast<TypeErasureImpl_*>(self); }
		};
	}


#define DETAIL_TYPEERASURE_CALL_PARAM_DECL(ParamIndex_, ParamVariantId_) \
	STINGRAYKIT_COMMA_IF(ParamIndex_) STINGRAYKIT_INSERT_IF(STINGRAYKIT_BITWISE_AND(ParamVariantId_, STINGRAYKIT_POW(2, ParamIndex_)), const) STINGRAYKIT_CAT(T, ParamIndex_)& STINGRAYKIT_CAT(p, ParamIndex_)

#define DETAIL_TYPEERASURE_CALL(N_, ParamsCount_) \
	template< typename Concept_ STINGRAYKIT_COMMA_IF(ParamsCount_) STINGRAYKIT_REPEAT_NESTING_2(ParamsCount_, STINGRAYKIT_TEMPLATE_PARAM_DECL, T) > \
	typename EnableIf<GetTypeListLength<typename Concept_::ParamTypes>::Value == N_, typename Concept_::RetType>::ValueT \
	Call(STINGRAYKIT_REPEAT_NESTING_2(ParamsCount_, DETAIL_TYPEERASURE_CALL_PARAM_DECL, N_)) \
	{ \
		typedef Tuple<typename Concept_::ParamTypes> ParamsTuple; \
		return CallPacked<Concept_>(ParamsTuple(STINGRAYKIT_REPEAT_NESTING_2(ParamsCount_, STINGRAYKIT_FUNCTION_PARAM_USAGE, ~))); \
	}


	template<typename Concepts_, typename Base_ = TypeErasureBase>
	class TypeErasure
	{
		typedef typename TypeListPrepend<Concepts_, Concepts::Destructor>::ValueT AllConcepts;

	private:
		Base_* _data;

	public:
		TypeErasure() : _data()
		{ }

		~TypeErasure()
		{ }

		template<typename Wrapped>
		void Allocate()
		{ _data = new TypeErasureImpl<AllConcepts, Wrapped>(); }

		template<typename Wrapped, typename T1>
		void Allocate(const T1& p1)
		{ _data = new TypeErasureImpl<AllConcepts, Wrapped>(p1); }

		template<typename Wrapped, typename T1, typename T2>
		void Allocate(const T1& p1, const T2& p2)
		{ _data = new TypeErasureImpl<AllConcepts, Wrapped>(p1, p2); }

		template<typename Wrapped, typename T1, typename T2, typename T3>
		void Allocate(const T1& p1, const T2& p2, const T3& p3)
		{ _data = new TypeErasureImpl<AllConcepts, Wrapped>(p1, p2, p3); }

		void Free()
		{ Call<Concepts::Destructor>(); _data = NULL; }

		Base_* Get() const
		{ return _data; }

		STINGRAYKIT_REPEAT( 1, DETAIL_TYPEERASURE_CALL, 0)
		STINGRAYKIT_REPEAT( 2, DETAIL_TYPEERASURE_CALL, 1)
		STINGRAYKIT_REPEAT( 4, DETAIL_TYPEERASURE_CALL, 2)
		STINGRAYKIT_REPEAT( 8, DETAIL_TYPEERASURE_CALL, 3)
		STINGRAYKIT_REPEAT(16, DETAIL_TYPEERASURE_CALL, 4)
		STINGRAYKIT_REPEAT(32, DETAIL_TYPEERASURE_CALL, 5)

		template<typename Concept_>
		typename Concept_::RetType CallPacked(const Tuple<typename Concept_::ParamTypes>& params)
		{
			STINGRAYKIT_ASSERT(_data);
			TypeErasureBase::VTableFunc* vTable = _data->GetVTable();
			STINGRAYKIT_ASSERT(vTable);

			const size_t ConceptIndex = IndexOfTypeListItem<AllConcepts, Concept_>::Value;
			TypeErasureBase::DetypedFunctionPtr virtualFunc = vTable(ConceptIndex);

			typedef typename Concept_::RetType TypedFunction(TypeErasureBase*, const Tuple<typename Concept_::ParamTypes>&);
			return reinterpret_cast<TypedFunction*>(virtualFunc)(_data, params);
		}
	};


#undef DETAIL_TYPEERASURE_CALL
#undef DETAIL_TYPEERASURE_CALL_PARAM_DECL


	template<typename Concepts_, typename Wrapped_>
	class TypeErasureImpl : public Wrapped_
	{
		typedef Wrapped_								Wrapped;
		typedef Concepts_								Concepts;
		typedef TypeErasureImpl<Concepts_, Wrapped_>	Self;

	public:
		TypeErasureImpl()
		{ TypeErasureBase::_vTable = &VTableFuncImpl; }

		template<typename T1>
		TypeErasureImpl(const T1& p1) : Wrapped_(p1)
		{ TypeErasureBase::_vTable = &VTableFuncImpl; }

		template<typename T1, typename T2>
		TypeErasureImpl(const T1& p1, const T2& p2) : Wrapped_(p1, p2)
		{ TypeErasureBase::_vTable = &VTableFuncImpl; }

		template<typename T1, typename T2, typename T3>
		TypeErasureImpl(const T1& p1, const T2& p2, const T3& p3) : Wrapped_(p1, p2, p3)
		{ TypeErasureBase::_vTable = &VTableFuncImpl; }

		~TypeErasureImpl()
		{ TypeErasureBase::_vTable = NULL; }

	private:
		typedef void DetypedFunction();
		typedef DetypedFunction* DetypedFunctionPtr;

		template<int Index>
		struct VTableHelper
		{
			static bool Call(size_t index, DetypedFunctionPtr& result)
			{
				if (index != Index)
					return true;
				typedef typename GetTypeListItem<Concepts, Index>::ValueT Concept;
				typedef Detail::ConceptInvoker<Self, Concept> ConceptInvoker;
				result = reinterpret_cast<DetypedFunctionPtr>(&ConceptInvoker::DoCall);
				return false; // stop ForIf
			}
		};

		static DetypedFunctionPtr VTableFuncImpl(size_t functionIndex)
		{
			DetypedFunctionPtr result = NULL;
			if (ForIf<GetTypeListLength<Concepts>::Value, VTableHelper>::Do(functionIndex, ref(result)))
				STINGRAYKIT_FATAL("Pure virtual function called");
			return result;
		}
	};

}

#endif
