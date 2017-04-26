#ifndef STINGRAYKIT_TYPESWITCH_H
#define STINGRAYKIT_TYPESWITCH_H

// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/Macro.h>

/**
 * @addtogroup toolkit_general
 * @{
 */

/**
 * @brief TypeSwitch - similar to switch, but takes types as cases. Can be used with variant or pointer to polymorphic class (using dynamic_cast)
 * @par Example:
 * @code
 *	struct OpFailed
 *	{
 *		std::string		Error;
 *	};
 *
 *	struct OpSucceeded
 *	{
 *		ByteArray	Result;
 *	};
 *
 *	typedef variant< TypeList<EmptyType, OpFailed, OpSucceeded> >		OpResult;
 *
 *	class Class
 *	{
 *		ByteArray	_data;
 *		IDialogPtr	_failDialog;
 *		...
 *
 *		void OperationHandler(const OpResult& result)
 *		{
 *			STINGRAYKIT_TYPE_SWITCH_VARIANT(result, value,
 *				STINGRAYKIT_TYPE_CASE(OpFailed,
 *					_failDialog->SetText("The operation failed, the error is: " + value.Error);
 *					_failDialog->Show();
 *					),
 *				STINGRAYKIT_TYPE_CASE(OpSucceeded,
 *					_data = value.Result;
 *					),
 *				STINGRAYKIT_TYPE_DEFAULT(
 *					STINGRAYKIT_THROW("Wrong OpResult!");
 *				)
 *			);
 *		}
 *	};
 * @endcode
 */

#define DETAIL_STINGRAYKIT_TYPE_CASE_IMPL(SwitchType_, N_, Value_, AsName_, Case_) \
	STINGRAYKIT_EXPAND(STINGRAYKIT_DEFER(DETAIL_STINGRAYKIT_TYPE_CASE_IMPL_WITH_MARKER)(SwitchType_, N_, Value_, AsName_, STINGRAYKIT_CAT(DETAIL_, Case_)))

#define DETAIL_STINGRAYKIT_TYPE_CASE_IMPL_WITH_MARKER(SwitchType_, N_, Value_, AsName_, Marker_, ...) \
	STINGRAYKIT_CAT(STINGRAYKIT_CAT(DETAIL_STINGRAYKIT_TYPE_CASE_IMPL_, SwitchType_), Marker_)(N_, Value_, AsName_, __VA_ARGS__)

#define DETAIL_STINGRAYKIT_TYPE_CASE_IMPL_DETAIL_VARIANTDETAIL_CASE(N_, Value_, AsName_, Type_, ...) \
	} else if (const Type_* detailTypeSwitchCase##N_ = Value_.get_ptr<Type_>()) { const Type_& AsName_ = *detailTypeSwitchCase##N_; __VA_ARGS__

#define DETAIL_STINGRAYKIT_TYPE_CASE_IMPL_DETAIL_VARIANTDETAIL_DEFAULT(N_, Value_, AsName_, ...) \
	} else { __VA_ARGS__

#define DETAIL_STINGRAYKIT_TYPE_CASE_IMPL_DETAIL_DYNAMICDETAIL_CASE(N_, Value_, AsName_, Type_, ...) \
	} else if (const Type_ detailTypeSwitchCase##N_ = DynamicCast<Type_>(Value_)) { const Type_& AsName_ = detailTypeSwitchCase##N_; __VA_ARGS__

#define DETAIL_STINGRAYKIT_TYPE_CASE_IMPL_DETAIL_DYNAMICDETAIL_DEFAULT(N_, Value_, AsName_, ...) \
	} else { __VA_ARGS__

#define DETAIL_STINGRAYKIT_TYPE_CASE(Type_, ...) DETAIL_CASE, Type_, __VA_ARGS__
#define DETAIL_STINGRAYKIT_TYPE_DEFAULT(...) DETAIL_DEFAULT, __VA_ARGS__

#define DETAIL_STINGRAYKIT_TYPE_SWITCH_IMPL_1(SwitchType_, Value_, AsName_, Case0_) \
	DETAIL_STINGRAYKIT_TYPE_CASE_IMPL(SwitchType_, 0, Value_, AsName_, Case0_)

#define DETAIL_STINGRAYKIT_TYPE_SWITCH_IMPL_2(SwitchType_, Value_, AsName_, Case0_, Case1_) \
	DETAIL_STINGRAYKIT_TYPE_CASE_IMPL(SwitchType_, 0, Value_, AsName_, Case0_) \
	DETAIL_STINGRAYKIT_TYPE_CASE_IMPL(SwitchType_, 1, Value_, AsName_, Case1_)

#define DETAIL_STINGRAYKIT_TYPE_SWITCH_IMPL_3(SwitchType_, Value_, AsName_, Case0_, Case1_, Case2_) \
	DETAIL_STINGRAYKIT_TYPE_CASE_IMPL(SwitchType_, 0, Value_, AsName_, Case0_) \
	DETAIL_STINGRAYKIT_TYPE_CASE_IMPL(SwitchType_, 1, Value_, AsName_, Case1_) \
	DETAIL_STINGRAYKIT_TYPE_CASE_IMPL(SwitchType_, 2, Value_, AsName_, Case2_)

#define DETAIL_STINGRAYKIT_TYPE_SWITCH_IMPL_4(SwitchType_, Value_, AsName_, Case0_, Case1_, Case2_, Case3_) \
	DETAIL_STINGRAYKIT_TYPE_CASE_IMPL(SwitchType_, 0, Value_, AsName_, Case0_) \
	DETAIL_STINGRAYKIT_TYPE_CASE_IMPL(SwitchType_, 1, Value_, AsName_, Case1_) \
	DETAIL_STINGRAYKIT_TYPE_CASE_IMPL(SwitchType_, 2, Value_, AsName_, Case2_) \
	DETAIL_STINGRAYKIT_TYPE_CASE_IMPL(SwitchType_, 3, Value_, AsName_, Case3_)

#define DETAIL_STINGRAYKIT_TYPE_SWITCH_IMPL_5(SwitchType_, Value_, AsName_, Case0_, Case1_, Case2_, Case3_, Case4_) \
	DETAIL_STINGRAYKIT_TYPE_CASE_IMPL(SwitchType_, 0, Value_, AsName_, Case0_) \
	DETAIL_STINGRAYKIT_TYPE_CASE_IMPL(SwitchType_, 1, Value_, AsName_, Case1_) \
	DETAIL_STINGRAYKIT_TYPE_CASE_IMPL(SwitchType_, 2, Value_, AsName_, Case2_) \
	DETAIL_STINGRAYKIT_TYPE_CASE_IMPL(SwitchType_, 3, Value_, AsName_, Case3_) \
	DETAIL_STINGRAYKIT_TYPE_CASE_IMPL(SwitchType_, 4, Value_, AsName_, Case4_)

#define DETAIL_STINGRAYKIT_TYPE_SWITCH_IMPL_6(SwitchType_, Value_, AsName_, Case0_, Case1_, Case2_, Case3_, Case4_, Case5_) \
	DETAIL_STINGRAYKIT_TYPE_CASE_IMPL(SwitchType_, 0, Value_, AsName_, Case0_) \
	DETAIL_STINGRAYKIT_TYPE_CASE_IMPL(SwitchType_, 1, Value_, AsName_, Case1_) \
	DETAIL_STINGRAYKIT_TYPE_CASE_IMPL(SwitchType_, 2, Value_, AsName_, Case2_) \
	DETAIL_STINGRAYKIT_TYPE_CASE_IMPL(SwitchType_, 3, Value_, AsName_, Case3_) \
	DETAIL_STINGRAYKIT_TYPE_CASE_IMPL(SwitchType_, 4, Value_, AsName_, Case4_) \
	DETAIL_STINGRAYKIT_TYPE_CASE_IMPL(SwitchType_, 5, Value_, AsName_, Case5_)

#define DETAIL_STINGRAYKIT_TYPE_SWITCH_IMPL_7(SwitchType_, Value_, AsName_, Case0_, Case1_, Case2_, Case3_, Case4_, Case5_, Case6_) \
	DETAIL_STINGRAYKIT_TYPE_CASE_IMPL(SwitchType_, 0, Value_, AsName_, Case0_) \
	DETAIL_STINGRAYKIT_TYPE_CASE_IMPL(SwitchType_, 1, Value_, AsName_, Case1_) \
	DETAIL_STINGRAYKIT_TYPE_CASE_IMPL(SwitchType_, 2, Value_, AsName_, Case2_) \
	DETAIL_STINGRAYKIT_TYPE_CASE_IMPL(SwitchType_, 3, Value_, AsName_, Case3_) \
	DETAIL_STINGRAYKIT_TYPE_CASE_IMPL(SwitchType_, 4, Value_, AsName_, Case4_) \
	DETAIL_STINGRAYKIT_TYPE_CASE_IMPL(SwitchType_, 5, Value_, AsName_, Case5_) \
	DETAIL_STINGRAYKIT_TYPE_CASE_IMPL(SwitchType_, 6, Value_, AsName_, Case6_)

#define DETAIL_STINGRAYKIT_TYPE_SWITCH_IMPL_8(SwitchType_, Value_, AsName_, Case0_, Case1_, Case2_, Case3_, Case4_, Case5_, Case6_, Case7_) \
	DETAIL_STINGRAYKIT_TYPE_CASE_IMPL(SwitchType_, 0, Value_, AsName_, Case0_) \
	DETAIL_STINGRAYKIT_TYPE_CASE_IMPL(SwitchType_, 1, Value_, AsName_, Case1_) \
	DETAIL_STINGRAYKIT_TYPE_CASE_IMPL(SwitchType_, 2, Value_, AsName_, Case2_) \
	DETAIL_STINGRAYKIT_TYPE_CASE_IMPL(SwitchType_, 3, Value_, AsName_, Case3_) \
	DETAIL_STINGRAYKIT_TYPE_CASE_IMPL(SwitchType_, 4, Value_, AsName_, Case4_) \
	DETAIL_STINGRAYKIT_TYPE_CASE_IMPL(SwitchType_, 5, Value_, AsName_, Case5_) \
	DETAIL_STINGRAYKIT_TYPE_CASE_IMPL(SwitchType_, 6, Value_, AsName_, Case6_) \
	DETAIL_STINGRAYKIT_TYPE_CASE_IMPL(SwitchType_, 7, Value_, AsName_, Case7_)

#define DETAIL_STINGRAYKIT_TYPE_SWITCH_IMPL_9(SwitchType_, Value_, AsName_, Case0_, Case1_, Case2_, Case3_, Case4_, Case5_, Case6_, Case7_, Case8_) \
	DETAIL_STINGRAYKIT_TYPE_CASE_IMPL(SwitchType_, 0, Value_, AsName_, Case0_) \
	DETAIL_STINGRAYKIT_TYPE_CASE_IMPL(SwitchType_, 1, Value_, AsName_, Case1_) \
	DETAIL_STINGRAYKIT_TYPE_CASE_IMPL(SwitchType_, 2, Value_, AsName_, Case2_) \
	DETAIL_STINGRAYKIT_TYPE_CASE_IMPL(SwitchType_, 3, Value_, AsName_, Case3_) \
	DETAIL_STINGRAYKIT_TYPE_CASE_IMPL(SwitchType_, 4, Value_, AsName_, Case4_) \
	DETAIL_STINGRAYKIT_TYPE_CASE_IMPL(SwitchType_, 5, Value_, AsName_, Case5_) \
	DETAIL_STINGRAYKIT_TYPE_CASE_IMPL(SwitchType_, 6, Value_, AsName_, Case6_) \
	DETAIL_STINGRAYKIT_TYPE_CASE_IMPL(SwitchType_, 7, Value_, AsName_, Case7_) \
	DETAIL_STINGRAYKIT_TYPE_CASE_IMPL(SwitchType_, 8, Value_, AsName_, Case8_)

#define DETAIL_STINGRAYKIT_TYPE_SWITCH_IMPL_10(SwitchType_, Value_, AsName_, Case0_, Case1_, Case2_, Case3_, Case4_, Case5_, Case6_, Case7_, Case8_, Case9_) \
	DETAIL_STINGRAYKIT_TYPE_CASE_IMPL(SwitchType_, 0, Value_, AsName_, Case0_) \
	DETAIL_STINGRAYKIT_TYPE_CASE_IMPL(SwitchType_, 1, Value_, AsName_, Case1_) \
	DETAIL_STINGRAYKIT_TYPE_CASE_IMPL(SwitchType_, 2, Value_, AsName_, Case2_) \
	DETAIL_STINGRAYKIT_TYPE_CASE_IMPL(SwitchType_, 3, Value_, AsName_, Case3_) \
	DETAIL_STINGRAYKIT_TYPE_CASE_IMPL(SwitchType_, 4, Value_, AsName_, Case4_) \
	DETAIL_STINGRAYKIT_TYPE_CASE_IMPL(SwitchType_, 5, Value_, AsName_, Case5_) \
	DETAIL_STINGRAYKIT_TYPE_CASE_IMPL(SwitchType_, 6, Value_, AsName_, Case6_) \
	DETAIL_STINGRAYKIT_TYPE_CASE_IMPL(SwitchType_, 7, Value_, AsName_, Case7_) \
	DETAIL_STINGRAYKIT_TYPE_CASE_IMPL(SwitchType_, 8, Value_, AsName_, Case8_) \
	DETAIL_STINGRAYKIT_TYPE_CASE_IMPL(SwitchType_, 9, Value_, AsName_, Case9_)

#define DETAIL_STINGRAYKIT_TYPE_SWITCH_VARIANT(Value_, AsName_, ...) \
	if (false) \
	{ \
		STINGRAYKIT_CAT(DETAIL_STINGRAYKIT_TYPE_SWITCH_IMPL_, STINGRAYKIT_NARGS(__VA_ARGS__))(DETAIL_VARIANT, Value_, AsName_, __VA_ARGS__) \
	}

#define DETAIL_STINGRAYKIT_TYPE_SWITCH_DYNAMIC(Value_, AsName_, ...) \
	if (false) \
	{ \
		STINGRAYKIT_CAT(DETAIL_STINGRAYKIT_TYPE_SWITCH_IMPL_, STINGRAYKIT_NARGS(__VA_ARGS__))(DETAIL_DYNAMIC, Value_, AsName_, __VA_ARGS__) \
	}

#define STINGRAYKIT_TYPE_SWITCH_VARIANT(...) DETAIL_STINGRAYKIT_TYPE_SWITCH_VARIANT(__VA_ARGS__)
#define STINGRAYKIT_TYPE_SWITCH_DYNAMIC(...) DETAIL_STINGRAYKIT_TYPE_SWITCH_DYNAMIC(__VA_ARGS__)

/** @} */

#endif
