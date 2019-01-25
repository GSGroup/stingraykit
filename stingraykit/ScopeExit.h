#ifndef STINGRAYKIT_SCOPEEXIT_H
#define STINGRAYKIT_SCOPEEXIT_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/log/Logger.h>
#include <stingraykit/function/function.h>
#include <stingraykit/Macro.h>
#include <stingraykit/optional.h>

namespace stingray
{

	/**
	 * @ingroup toolkit_general
	 * @defgroup toolkit_general_scopeexit Scope exit
	 * @{
	 */


	/**
	 * @brief Start of the code that should be executed at the scope exit
	 * @param[in] ... Each parameter is an MK_PARAM pair of the type and the name of a closure variable.
	 * @par Example:
	 * @code
	 *	void func(int file)
	 *	{
	 *		size_t length = 0x1000
	 *		void* ptr = mmap(NULL, length, PROT_NONE, 0, file, 12345);
	 *
	 *		STINGRAYKIT_SCOPE_EXIT(MK_PARAM(void*, ptr), MK_PARAM(size_t, length))
	 *			munmap(ptr, length);
	 *		STINGRAYKIT_SCOPE_EXIT_END;
	 *
	 *		// Working with ptr...
	 *	}
	 * @endcode
	 */
#define STINGRAYKIT_SCOPE_EXIT(...) \
		STINGRAYKIT_CAT(STINGRAYKIT_DECLARE_SCOPE_EXIT_ARGS_, STINGRAYKIT_NARGS(__VA_ARGS__))(__VA_ARGS__) \
		class STINGRAYKIT_CAT(__ScopeExitFunc, __LINE__) : protected STINGRAYKIT_CAT(__ScopeExitArgs, __LINE__) \
		{ \
		public: \
			STINGRAYKIT_CAT(__ScopeExitFunc, __LINE__) (STINGRAYKIT_CAT(__ScopeExitArgs, __LINE__) args) : STINGRAYKIT_CAT(__ScopeExitArgs, __LINE__)(args) { } \
			~STINGRAYKIT_CAT(__ScopeExitFunc, __LINE__) () { STINGRAYKIT_TRY("Unhandled exception in " TO_STRING(STINGRAYKIT_CAT(__ScopeExitFunc, __LINE__)), Dtor()); } \
			\
			void Dtor() {

	/** @brief End of the code that should be executed at the scope exit */
#define STINGRAYKIT_SCOPE_EXIT_END \
			} \
		} __scope_exit(__scope_exit_args);



	/** @brief a helper class that invokes a function in its destructor */
	class ScopeExitInvoker
	{
		STINGRAYKIT_NONCOPYABLE(ScopeExitInvoker);

	private:
		typedef function<void ()>	FuncType;

		optional<FuncType>			_func;

	public:
		/** @param[in] func Function that should be invoked from the ScopeExitInvoker destructor */
		ScopeExitInvoker(const FuncType& func)
			:	_func(func)
		{ }

		~ScopeExitInvoker()
		{
			if (_func)
				STINGRAYKIT_TRY("Unhandled exception in ScopeExitInvoker!", (*_func)());
		}

		void Cancel()
		{ _func.reset(); }
	};

	/** @} */

#define STINGRAYKIT_DECLARE_SCOPE_EXIT_ARGS_0() \
		struct STINGRAYKIT_CAT(__ScopeExitArgs, __LINE__) \
		{ \
		} __scope_exit_args; \

#define STINGRAYKIT_DECLARE_SCOPE_EXIT_ARGS_2(ParamType1_, ParamName1_) \
		struct STINGRAYKIT_CAT(__ScopeExitArgs, __LINE__) \
		{ \
			ParamType1_		ParamName1_; \
			STINGRAYKIT_CAT(__ScopeExitArgs, __LINE__)(ParamType1_ ParamName1_) : ParamName1_(ParamName1_) { } \
		} __scope_exit_args(ParamName1_); \

#define STINGRAYKIT_DECLARE_SCOPE_EXIT_ARGS_4(ParamType1_, ParamName1_, ParamType2_, ParamName2_) \
		struct STINGRAYKIT_CAT(__ScopeExitArgs, __LINE__) \
		{ \
			ParamType1_		ParamName1_; \
			ParamType2_		ParamName2_; \
			STINGRAYKIT_CAT(__ScopeExitArgs, __LINE__)(ParamType1_ ParamName1_, ParamType2_ ParamName2_) : ParamName1_(ParamName1_), ParamName2_(ParamName2_) { } \
		} __scope_exit_args(ParamName1_, ParamName2_); \

#define STINGRAYKIT_DECLARE_SCOPE_EXIT_ARGS_6(ParamType1_, ParamName1_, ParamType2_, ParamName2_, ParamType3_, ParamName3_) \
		struct STINGRAYKIT_CAT(__ScopeExitArgs, __LINE__) \
		{ \
			ParamType1_		ParamName1_; \
			ParamType2_		ParamName2_; \
			ParamType3_		ParamName3_; \
			STINGRAYKIT_CAT(__ScopeExitArgs, __LINE__)(ParamType1_ ParamName1_, ParamType2_ ParamName2_, ParamType3_ ParamName3_) : ParamName1_(ParamName1_), ParamName2_(ParamName2_), ParamName3_(ParamName3_) { } \
		} __scope_exit_args(ParamName1_, ParamName2_, ParamName3_);

#define STINGRAYKIT_DECLARE_SCOPE_EXIT_ARGS_8(ParamType1_, ParamName1_, ParamType2_, ParamName2_, ParamType3_, ParamName3_, ParamType4_, ParamName4_) \
		struct STINGRAYKIT_CAT(__ScopeExitArgs, __LINE__) \
		{ \
			ParamType1_		ParamName1_; \
			ParamType2_		ParamName2_; \
			ParamType3_		ParamName3_; \
			ParamType4_		ParamName4_; \
			STINGRAYKIT_CAT(__ScopeExitArgs, __LINE__)(ParamType1_ ParamName1_, ParamType2_ ParamName2_, ParamType3_ ParamName3_, ParamType4_ ParamName4_) : ParamName1_(ParamName1_), ParamName2_(ParamName2_), ParamName3_(ParamName3_), ParamName4_(ParamName4_) { } \
		} __scope_exit_args(ParamName1_, ParamName2_, ParamName3_, ParamName4_);

#define STINGRAYKIT_DECLARE_SCOPE_EXIT_ARGS_10(ParamType1_, ParamName1_, ParamType2_, ParamName2_, ParamType3_, ParamName3_, ParamType4_, ParamName4_, ParamType5_, ParamName5_) \
		struct STINGRAYKIT_CAT(__ScopeExitArgs, __LINE__) \
		{ \
			ParamType1_		ParamName1_; \
			ParamType2_		ParamName2_; \
			ParamType3_		ParamName3_; \
			ParamType4_		ParamName4_; \
			ParamType5_		ParamName5_; \
			STINGRAYKIT_CAT(__ScopeExitArgs, __LINE__)(ParamType1_ ParamName1_, ParamType2_ ParamName2_, ParamType3_ ParamName3_, ParamType4_ ParamName4_, ParamType5_ ParamName5_) : ParamName1_(ParamName1_), ParamName2_(ParamName2_), ParamName3_(ParamName3_), ParamName4_(ParamName4_), ParamName5_(ParamName5_) { } \
		} __scope_exit_args(ParamName1_, ParamName2_, ParamName3_, ParamName4_, ParamName5_);

#define STINGRAYKIT_DECLARE_SCOPE_EXIT_ARGS_12(ParamType1_, ParamName1_, ParamType2_, ParamName2_, ParamType3_, ParamName3_, ParamType4_, ParamName4_, ParamType5_, ParamName5_, ParamType6_, ParamName6_) \
		struct STINGRAYKIT_CAT(__ScopeExitArgs, __LINE__) \
		{ \
			ParamType1_		ParamName1_; \
			ParamType2_		ParamName2_; \
			ParamType3_		ParamName3_; \
			ParamType4_		ParamName4_; \
			ParamType5_		ParamName5_; \
			ParamType6_		ParamName6_; \
			STINGRAYKIT_CAT(__ScopeExitArgs, __LINE__)(ParamType1_ ParamName1_, ParamType2_ ParamName2_, ParamType3_ ParamName3_, ParamType4_ ParamName4_, ParamType5_ ParamName5_, ParamType6_ ParamName6_) : ParamName1_(ParamName1_), ParamName2_(ParamName2_), ParamName3_(ParamName3_), ParamName4_(ParamName4_), ParamName5_(ParamName5_), ParamName6_(ParamName6_) { } \
		} __scope_exit_args(ParamName1_, ParamName2_, ParamName3_, ParamName4_, ParamName5_, ParamName6_);

#define STINGRAYKIT_DECLARE_SCOPE_EXIT_ARGS_14(ParamType1_, ParamName1_, ParamType2_, ParamName2_, ParamType3_, ParamName3_, ParamType4_, ParamName4_, ParamType5_, ParamName5_, ParamType6_, ParamName6_, ParamType7_, ParamName7_) \
		struct STINGRAYKIT_CAT(__ScopeExitArgs, __LINE__) \
		{ \
			ParamType1_		ParamName1_; \
			ParamType2_		ParamName2_; \
			ParamType3_		ParamName3_; \
			ParamType4_		ParamName4_; \
			ParamType5_		ParamName5_; \
			ParamType6_		ParamName6_; \
			ParamType7_		ParamName7_; \
			STINGRAYKIT_CAT(__ScopeExitArgs, __LINE__)(ParamType1_ ParamName1_, ParamType2_ ParamName2_, ParamType3_ ParamName3_, ParamType4_ ParamName4_, ParamType5_ ParamName5_, ParamType6_ ParamName6_, ParamType7_ ParamName7_) : ParamName1_(ParamName1_), ParamName2_(ParamName2_), ParamName3_(ParamName3_), ParamName4_(ParamName4_), ParamName5_(ParamName5_), ParamName6_(ParamName6_), ParamName7_(ParamName7_) { } \
		} __scope_exit_args(ParamName1_, ParamName2_, ParamName3_, ParamName4_, ParamName5_, ParamName6_, ParamName7_);

#define STINGRAYKIT_DECLARE_SCOPE_EXIT_ARGS_16(ParamType1_, ParamName1_, ParamType2_, ParamName2_, ParamType3_, ParamName3_, ParamType4_, ParamName4_, ParamType5_, ParamName5_, ParamType6_, ParamName6_, ParamType7_, ParamName7_, ParamType8_, ParamName8_) \
		struct STINGRAYKIT_CAT(__ScopeExitArgs, __LINE__) \
		{ \
			ParamType1_		ParamName1_; \
			ParamType2_		ParamName2_; \
			ParamType3_		ParamName3_; \
			ParamType4_		ParamName4_; \
			ParamType5_		ParamName5_; \
			ParamType6_		ParamName6_; \
			ParamType7_		ParamName7_; \
			ParamType8_		ParamName8_; \
			STINGRAYKIT_CAT(__ScopeExitArgs, __LINE__)(ParamType1_ ParamName1_, ParamType2_ ParamName2_, ParamType3_ ParamName3_, ParamType4_ ParamName4_, ParamType5_ ParamName5_, ParamType6_ ParamName6_, ParamType7_ ParamName7_, ParamType8_ ParamName8_) : ParamName1_(ParamName1_), ParamName2_(ParamName2_), ParamName3_(ParamName3_), ParamName4_(ParamName4_), ParamName5_(ParamName5_), ParamName6_(ParamName6_), ParamName7_(ParamName7_), ParamName8_(ParamName8_) { } \
		} __scope_exit_args(ParamName1_, ParamName2_, ParamName3_, ParamName4_, ParamName5_, ParamName6_, ParamName7_, ParamName8_);

#define STINGRAYKIT_DECLARE_SCOPE_EXIT_ARGS_18(ParamType1_, ParamName1_, ParamType2_, ParamName2_, ParamType3_, ParamName3_, ParamType4_, ParamName4_, ParamType5_, ParamName5_, ParamType6_, ParamName6_, ParamType7_, ParamName7_, ParamType8_, ParamName8_, ParamType9_, ParamName9_) \
		struct STINGRAYKIT_CAT(__ScopeExitArgs, __LINE__) \
		{ \
			ParamType1_		ParamName1_; \
			ParamType2_		ParamName2_; \
			ParamType3_		ParamName3_; \
			ParamType4_		ParamName4_; \
			ParamType5_		ParamName5_; \
			ParamType6_		ParamName6_; \
			ParamType7_		ParamName7_; \
			ParamType8_		ParamName8_; \
			ParamType9_		ParamName9_; \
			STINGRAYKIT_CAT(__ScopeExitArgs, __LINE__)(ParamType1_ ParamName1_, ParamType2_ ParamName2_, ParamType3_ ParamName3_, ParamType4_ ParamName4_, ParamType5_ ParamName5_, ParamType6_ ParamName6_, ParamType7_ ParamName7_, ParamType8_ ParamName8_, ParamType9_ ParamName9_) : ParamName1_(ParamName1_), ParamName2_(ParamName2_), ParamName3_(ParamName3_), ParamName4_(ParamName4_), ParamName5_(ParamName5_), ParamName6_(ParamName6_), ParamName7_(ParamName7_), ParamName8_(ParamName8_), ParamName9_(ParamName9_) { } \
		} __scope_exit_args(ParamName1_, ParamName2_, ParamName3_, ParamName4_, ParamName5_, ParamName6_, ParamName7_, ParamName8_, ParamName9_);

#define STINGRAYKIT_DECLARE_SCOPE_EXIT_ARGS_20(ParamType1_, ParamName1_, ParamType2_, ParamName2_, ParamType3_, ParamName3_, ParamType4_, ParamName4_, ParamType5_, ParamName5_, ParamType6_, ParamName6_, ParamType7_, ParamName7_, ParamType8_, ParamName8_, ParamType9_, ParamName9_, ParamType10_, ParamName10_) \
		struct STINGRAYKIT_CAT(__ScopeExitArgs, __LINE__) \
		{ \
			ParamType1_		ParamName1_; \
			ParamType2_		ParamName2_; \
			ParamType3_		ParamName3_; \
			ParamType4_		ParamName4_; \
			ParamType5_		ParamName5_; \
			ParamType6_		ParamName6_; \
			ParamType7_		ParamName7_; \
			ParamType8_		ParamName8_; \
			ParamType9_		ParamName9_; \
			ParamType10_		ParamName10_; \
			STINGRAYKIT_CAT(__ScopeExitArgs, __LINE__)(ParamType1_ ParamName1_, ParamType2_ ParamName2_, ParamType3_ ParamName3_, ParamType4_ ParamName4_, ParamType5_ ParamName5_, ParamType6_ ParamName6_, ParamType7_ ParamName7_, ParamType8_ ParamName8_, ParamType9_ ParamName9_, ParamType10_ ParamName10_) : ParamName1_(ParamName1_), ParamName2_(ParamName2_), ParamName3_(ParamName3_), ParamName4_(ParamName4_), ParamName5_(ParamName5_), ParamName6_(ParamName6_), ParamName7_(ParamName7_), ParamName8_(ParamName8_), ParamName9_(ParamName9_), ParamName10_(ParamName10_) { } \
		} __scope_exit_args(ParamName1_, ParamName2_, ParamName3_, ParamName4_, ParamName5_, ParamName6_, ParamName7_, ParamName8_, ParamName9_, ParamName10_);

}



#endif
