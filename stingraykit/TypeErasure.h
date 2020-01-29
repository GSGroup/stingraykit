#ifndef STINGRAYKIT_TYPEERASURE_H
#define STINGRAYKIT_TYPEERASURE_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/function/function_info.h>
#include <stingraykit/fatal.h>
#include <stingraykit/Macro.h>
#include <stingraykit/PerfectForwarding.h>
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
		template < typename TypeErasureImpl_, typename Concept_, size_t ParamsCount = GetTypeListLength<typename Concept_::ParamTypes>::Value >
		struct ConceptInvoker;


#define DETAIL_CONCEPTINVOKER_PARAM(Index_, UserArg_) STINGRAYKIT_COMMA_IF(Index_) typename GetTypeListItem<typename Concept_::ParamTypes, Index_>::ValueT p##Index_
#define DETAIL_CONCEPTINVOKER(N_) \
		template<typename TypeErasureImpl_, typename Concept_> \
		struct ConceptInvoker<TypeErasureImpl_, Concept_, N_> \
		{ \
			static typename Concept_::RetType DoCall(TypeErasureBase* self, STINGRAYKIT_REPEAT(N_, DETAIL_CONCEPTINVOKER_PARAM, ~)) \
			{ return Concept_::Apply(static_cast<TypeErasureImpl_&>(*self), STINGRAYKIT_REPEAT(N_, STINGRAYKIT_FUNCTION_PARAM_USAGE, ~)); } \
		};


		template < typename TypeErasureImpl_, typename Concept_ >
		struct ConceptInvoker<TypeErasureImpl_, Concept_, 0>
		{
			static typename Concept_::RetType DoCall(TypeErasureBase* self)
			{ return Concept_::Apply(static_cast<TypeErasureImpl_&>(*self)); }
		};


		DETAIL_CONCEPTINVOKER(1)
		DETAIL_CONCEPTINVOKER(2)
		DETAIL_CONCEPTINVOKER(3)
		DETAIL_CONCEPTINVOKER(4)
		DETAIL_CONCEPTINVOKER(5)
		DETAIL_CONCEPTINVOKER(6)
		DETAIL_CONCEPTINVOKER(7)
		DETAIL_CONCEPTINVOKER(8)
		DETAIL_CONCEPTINVOKER(9)
		DETAIL_CONCEPTINVOKER(10)


		template < typename TypeErasureImpl_ >
		struct ConceptInvoker<TypeErasureImpl_, Concepts::Destructor, 0>
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


#define DETAIL_TYPEERASURE_CALL_PARAM_DECL(ParamIndex_, ParamVariantId_) \
	STINGRAYKIT_COMMA_IF(ParamIndex_) STINGRAYKIT_INSERT_IF(STINGRAYKIT_BITWISE_AND(ParamVariantId_, STINGRAYKIT_POW(2, ParamIndex_)), const) STINGRAYKIT_CAT(T, ParamIndex_)& STINGRAYKIT_CAT(p, ParamIndex_)

#define DETAIL_TYPEERASURE_CALL(N_, ParamsCount_) \
	template< typename Concept_ STINGRAYKIT_COMMA_IF(ParamsCount_) STINGRAYKIT_REPEAT_NESTING_2(ParamsCount_, STINGRAYKIT_TEMPLATE_PARAM_DECL, T) > \
	typename Concept_::RetType Call(STINGRAYKIT_REPEAT_NESTING_2(ParamsCount_, DETAIL_TYPEERASURE_CALL_PARAM_DECL, N_)) \
	{ \
		CompileTimeAssert<GetTypeListLength<typename Concept_::ParamTypes>::Value == N_> ErrorParamsCountMismatch; (void)ErrorParamsCountMismatch; \
		TypeErasureBase::VTableFunc* vTable = _data->GetVTable(); \
		const size_t ConceptIndex = IndexOfTypeListItem<AllConcepts, Concept_>::Value; \
		TypeErasureBase::DetypedFunctionPtr virtualFunc = vTable(ConceptIndex); \
		typedef typename TypeListPrepend<typename Concept_::ParamTypes, TypeErasureBase*>::ValueT AllParams; \
		typedef typename SignatureBuilder<typename Concept_::RetType, AllParams>::ValueT Signature; \
		return reinterpret_cast<Signature*>(virtualFunc)(_data STINGRAYKIT_COMMA_IF(ParamsCount_) STINGRAYKIT_REPEAT_NESTING_2(ParamsCount_, STINGRAYKIT_FUNCTION_PARAM_USAGE, ~)); \
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

		~TypeErasure()
		{ }

		TypeErasure& operator = (const TypeErasure& other)
		{ _data = other._data; return *this; }

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

		STINGRAYKIT_REPEAT( 1, DETAIL_TYPEERASURE_CALL, 0)
		STINGRAYKIT_REPEAT( 2, DETAIL_TYPEERASURE_CALL, 1)
		STINGRAYKIT_REPEAT( 4, DETAIL_TYPEERASURE_CALL, 2)
		STINGRAYKIT_REPEAT( 8, DETAIL_TYPEERASURE_CALL, 3)
		STINGRAYKIT_REPEAT(16, DETAIL_TYPEERASURE_CALL, 4)
		STINGRAYKIT_REPEAT(32, DETAIL_TYPEERASURE_CALL, 5)
	};


#undef DETAIL_TYPEERASURE_CALL
#undef DETAIL_TYPEERASURE_CALL_PARAM_DECL


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
				typedef Detail::ConceptInvoker<Self, Concept> ConceptInvoker;
				result = reinterpret_cast<DetypedFunctionPtr>(&ConceptInvoker::DoCall);
				return false;
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
