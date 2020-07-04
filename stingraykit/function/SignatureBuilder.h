#ifndef STINGRAYKIT_FUNCTION_SIGNATUREBUILDER_H
#define STINGRAYKIT_FUNCTION_SIGNATUREBUILDER_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/Macro.h>
#include <stingraykit/toolkit.h>

namespace stingray
{

	/**
	 * @addtogroup toolkit_functions
	 * @{
	 */

	struct UnspecifiedRetType;
	struct UnspecifiedParamTypes;


	namespace Detail
	{

		template < size_t ParamsCount, typename RetType, typename ParamTypes >
		struct SignatureBuilderImpl;

#define DETAIL_STINGRAYKIT_DECLARE_SIGNATURE_BUILDER_ENUM_PARAMS(ParamNumber_, TypeListName_) STINGRAYKIT_COMMA_IF(ParamNumber_) typename GetTypeListItem<TypeListName_, ParamNumber_>::ValueT
#define DETAIL_STINGRAYKIT_DECLARE_SIGNATURE_BUILDER(ParamsCount_, UserData_) \
		template < typename RetType, typename ParamTypes > \
		struct SignatureBuilderImpl<ParamsCount_, RetType, ParamTypes> \
		{ \
			typedef RetType 		ValueT(STINGRAYKIT_REPEAT(ParamsCount_, DETAIL_STINGRAYKIT_DECLARE_SIGNATURE_BUILDER_ENUM_PARAMS, ParamTypes)); \
		};

		STINGRAYKIT_REPEAT_NESTING_2(10, DETAIL_STINGRAYKIT_DECLARE_SIGNATURE_BUILDER, ~)

#undef DETAIL_STINGRAYKIT_DECLARE_SIGNATURE_BUILDER
#undef DETAIL_STINGRAYKIT_DECLARE_SIGNATURE_BUILDER_ENUM_PARAMS

	}


	template < typename RetType, typename ParamTypes, bool AllSpecified = !IsSame<RetType, UnspecifiedRetType>::Value && !IsSame<ParamTypes, UnspecifiedParamTypes>::Value >
	struct SignatureBuilder
	{ typedef typename Detail::SignatureBuilderImpl<GetTypeListLength<ParamTypes>::Value, RetType, ParamTypes>::ValueT ValueT; };


	template < typename RetType, typename ParamTypes >
	struct SignatureBuilder<RetType, ParamTypes, false>
	{ typedef NullType ValueT; };

	/** @} */

}

#endif
