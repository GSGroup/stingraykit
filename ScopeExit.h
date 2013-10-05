#ifndef STINGRAY_TOOLKIT_SCOPEEXIT_H
#define STINGRAY_TOOLKIT_SCOPEEXIT_H


#include <stingray/toolkit/toolkit.h>
#include <stingray/toolkit/function.h>
#include <stingray/toolkit/Macro.h>


/**
 * Example:
 *
 *	void func(int file)
 *	{
 *		size_t length = 0x1000
 *		void* ptr = mmap(NULL, length, PROT_NONE, 0, file, 12345);
 *
 *		STINGRAY_SCOPE_EXIT(MK_PARAM(void*, ptr), MK_PARAM(size_t, length))
 *			munmap(ptr, length);
 *		STINGRAY_SCOPE_EXIT_END;
 *
 *		// Working with ptr...
 *	}
 *
 */


/*! \cond GS_INTERNAL */

namespace stingray
{

	class ScopeExitInvoker
	{
	private:
		TOOLKIT_NONCOPYABLE(ScopeExitInvoker);
		function<void()>	_func;

	public:
		ScopeExitInvoker(const function<void()>& func)
			: _func(func)
		{ }

		~ScopeExitInvoker()
		{ _func(); }
	};

#define STINGRAY_DECLARE_SCOPE_EXIT_ARGS_2(ParamType1_, ParamName1_) \
		struct TOOLKIT_CAT(__ScopeExitArgs, __LINE__) \
		{ \
			ParamType1_		ParamName1_; \
			TOOLKIT_CAT(__ScopeExitArgs, __LINE__)(ParamType1_ ParamName1_) : ParamName1_(ParamName1_) { } \
		} __scope_exit_args(ParamName1_); \

#define STINGRAY_DECLARE_SCOPE_EXIT_ARGS_4(ParamType1_, ParamName1_, ParamType2_, ParamName2_) \
		struct TOOLKIT_CAT(__ScopeExitArgs, __LINE__) \
		{ \
			ParamType1_		ParamName1_; \
			ParamType2_		ParamName2_; \
			TOOLKIT_CAT(__ScopeExitArgs, __LINE__)(ParamType1_ ParamName1_, ParamType2_ ParamName2_) : ParamName1_(ParamName1_), ParamName2_(ParamName2_) { } \
		} __scope_exit_args(ParamName1_, ParamName2_); \

#define STINGRAY_DECLARE_SCOPE_EXIT_ARGS_6(ParamType1_, ParamName1_, ParamType2_, ParamName2_, ParamType3_, ParamName3_) \
		struct TOOLKIT_CAT(__ScopeExitArgs, __LINE__) \
		{ \
			ParamType1_		ParamName1_; \
			ParamType2_		ParamName2_; \
			ParamType3_		ParamName3_; \
			TOOLKIT_CAT(__ScopeExitArgs, __LINE__)(ParamType1_ ParamName1_, ParamType2_ ParamName2_, ParamType3_ ParamName3_) : ParamName1_(ParamName1_), ParamName2_(ParamName2_), ParamName3_(ParamName3_) { } \
		} __scope_exit_args(ParamName1_, ParamName2_, ParamName3_);

#define STINGRAY_DECLARE_SCOPE_EXIT_ARGS_8(ParamType1_, ParamName1_, ParamType2_, ParamName2_, ParamType3_, ParamName3_, ParamType4_, ParamName4_) \
		struct TOOLKIT_CAT(__ScopeExitArgs, __LINE__) \
		{ \
			ParamType1_		ParamName1_; \
			ParamType2_		ParamName2_; \
			ParamType3_		ParamName3_; \
			ParamType4_		ParamName4_; \
			TOOLKIT_CAT(__ScopeExitArgs, __LINE__)(ParamType1_ ParamName1_, ParamType2_ ParamName2_, ParamType3_ ParamName3_, ParamType4_ ParamName4_) : ParamName1_(ParamName1_), ParamName2_(ParamName2_), ParamName3_(ParamName3_), ParamName4_(ParamName4_) { } \
		} __scope_exit_args(ParamName1_, ParamName2_, ParamName3_, ParamName4_);

#define STINGRAY_DECLARE_SCOPE_EXIT_ARGS_10(ParamType1_, ParamName1_, ParamType2_, ParamName2_, ParamType3_, ParamName3_, ParamType4_, ParamName4_, ParamType5_, ParamName5_) \
		struct TOOLKIT_CAT(__ScopeExitArgs, __LINE__) \
		{ \
			ParamType1_		ParamName1_; \
			ParamType2_		ParamName2_; \
			ParamType3_		ParamName3_; \
			ParamType4_		ParamName4_; \
			ParamType5_		ParamName5_; \
			TOOLKIT_CAT(__ScopeExitArgs, __LINE__)(ParamType1_ ParamName1_, ParamType2_ ParamName2_, ParamType3_ ParamName3_, ParamType4_ ParamName4_, ParamType5_ ParamName5_) : ParamName1_(ParamName1_), ParamName2_(ParamName2_), ParamName3_(ParamName3_), ParamName4_(ParamName4_), ParamName5_(ParamName5_) { } \
		} __scope_exit_args(ParamName1_, ParamName2_, ParamName3_, ParamName4_, ParamName5_);

#define STINGRAY_DECLARE_SCOPE_EXIT_ARGS_12(ParamType1_, ParamName1_, ParamType2_, ParamName2_, ParamType3_, ParamName3_, ParamType4_, ParamName4_, ParamType5_, ParamName5_, ParamType6_, ParamName6_) \
		struct TOOLKIT_CAT(__ScopeExitArgs, __LINE__) \
		{ \
			ParamType1_		ParamName1_; \
			ParamType2_		ParamName2_; \
			ParamType3_		ParamName3_; \
			ParamType4_		ParamName4_; \
			ParamType5_		ParamName5_; \
			ParamType6_		ParamName6_; \
			TOOLKIT_CAT(__ScopeExitArgs, __LINE__)(ParamType1_ ParamName1_, ParamType2_ ParamName2_, ParamType3_ ParamName3_, ParamType4_ ParamName4_, ParamType5_ ParamName5_, ParamType6_ ParamName6_) : ParamName1_(ParamName1_), ParamName2_(ParamName2_), ParamName3_(ParamName3_), ParamName4_(ParamName4_), ParamName5_(ParamName5_), ParamName6_(ParamName6_) { } \
		} __scope_exit_args(ParamName1_, ParamName2_, ParamName3_, ParamName4_, ParamName5_, ParamName6_);

#define STINGRAY_DECLARE_SCOPE_EXIT_ARGS_14(ParamType1_, ParamName1_, ParamType2_, ParamName2_, ParamType3_, ParamName3_, ParamType4_, ParamName4_, ParamType5_, ParamName5_, ParamType6_, ParamName6_, ParamType7_, ParamName7_) \
		struct TOOLKIT_CAT(__ScopeExitArgs, __LINE__) \
		{ \
			ParamType1_		ParamName1_; \
			ParamType2_		ParamName2_; \
			ParamType3_		ParamName3_; \
			ParamType4_		ParamName4_; \
			ParamType5_		ParamName5_; \
			ParamType6_		ParamName6_; \
			ParamType7_		ParamName7_; \
			TOOLKIT_CAT(__ScopeExitArgs, __LINE__)(ParamType1_ ParamName1_, ParamType2_ ParamName2_, ParamType3_ ParamName3_, ParamType4_ ParamName4_, ParamType5_ ParamName5_, ParamType6_ ParamName6_, ParamType7_ ParamName7_) : ParamName1_(ParamName1_), ParamName2_(ParamName2_), ParamName3_(ParamName3_), ParamName4_(ParamName4_), ParamName5_(ParamName5_), ParamName6_(ParamName6_), ParamName7_(ParamName7_) { } \
		} __scope_exit_args(ParamName1_, ParamName2_, ParamName3_, ParamName4_, ParamName5_, ParamName6_, ParamName7_);

#define STINGRAY_DECLARE_SCOPE_EXIT_ARGS_16(ParamType1_, ParamName1_, ParamType2_, ParamName2_, ParamType3_, ParamName3_, ParamType4_, ParamName4_, ParamType5_, ParamName5_, ParamType6_, ParamName6_, ParamType7_, ParamName7_, ParamType8_, ParamName8_) \
		struct TOOLKIT_CAT(__ScopeExitArgs, __LINE__) \
		{ \
			ParamType1_		ParamName1_; \
			ParamType2_		ParamName2_; \
			ParamType3_		ParamName3_; \
			ParamType4_		ParamName4_; \
			ParamType5_		ParamName5_; \
			ParamType6_		ParamName6_; \
			ParamType7_		ParamName7_; \
			ParamType8_		ParamName8_; \
			TOOLKIT_CAT(__ScopeExitArgs, __LINE__)(ParamType1_ ParamName1_, ParamType2_ ParamName2_, ParamType3_ ParamName3_, ParamType4_ ParamName4_, ParamType5_ ParamName5_, ParamType6_ ParamName6_, ParamType7_ ParamName7_, ParamType8_ ParamName8_) : ParamName1_(ParamName1_), ParamName2_(ParamName2_), ParamName3_(ParamName3_), ParamName4_(ParamName4_), ParamName5_(ParamName5_), ParamName6_(ParamName6_), ParamName7_(ParamName7_), ParamName8_(ParamName8_) { } \
		} __scope_exit_args(ParamName1_, ParamName2_, ParamName3_, ParamName4_, ParamName5_, ParamName6_, ParamName7_, ParamName8_);

#define STINGRAY_DECLARE_SCOPE_EXIT_ARGS_18(ParamType1_, ParamName1_, ParamType2_, ParamName2_, ParamType3_, ParamName3_, ParamType4_, ParamName4_, ParamType5_, ParamName5_, ParamType6_, ParamName6_, ParamType7_, ParamName7_, ParamType8_, ParamName8_, ParamType9_, ParamName9_) \
		struct TOOLKIT_CAT(__ScopeExitArgs, __LINE__) \
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
			TOOLKIT_CAT(__ScopeExitArgs, __LINE__)(ParamType1_ ParamName1_, ParamType2_ ParamName2_, ParamType3_ ParamName3_, ParamType4_ ParamName4_, ParamType5_ ParamName5_, ParamType6_ ParamName6_, ParamType7_ ParamName7_, ParamType8_ ParamName8_, ParamType9_ ParamName9_) : ParamName1_(ParamName1_), ParamName2_(ParamName2_), ParamName3_(ParamName3_), ParamName4_(ParamName4_), ParamName5_(ParamName5_), ParamName6_(ParamName6_), ParamName7_(ParamName7_), ParamName8_(ParamName8_), ParamName9_(ParamName9_) { } \
		} __scope_exit_args(ParamName1_, ParamName2_, ParamName3_, ParamName4_, ParamName5_, ParamName6_, ParamName7_, ParamName8_, ParamName9_);

#define STINGRAY_DECLARE_SCOPE_EXIT_ARGS_20(ParamType1_, ParamName1_, ParamType2_, ParamName2_, ParamType3_, ParamName3_, ParamType4_, ParamName4_, ParamType5_, ParamName5_, ParamType6_, ParamName6_, ParamType7_, ParamName7_, ParamType8_, ParamName8_, ParamType9_, ParamName9_, ParamType10_, ParamName10_) \
		struct TOOLKIT_CAT(__ScopeExitArgs, __LINE__) \
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
			TOOLKIT_CAT(__ScopeExitArgs, __LINE__)(ParamType1_ ParamName1_, ParamType2_ ParamName2_, ParamType3_ ParamName3_, ParamType4_ ParamName4_, ParamType5_ ParamName5_, ParamType6_ ParamName6_, ParamType7_ ParamName7_, ParamType8_ ParamName8_, ParamType9_ ParamName9_, ParamType10_ ParamName10_) : ParamName1_(ParamName1_), ParamName2_(ParamName2_), ParamName3_(ParamName3_), ParamName4_(ParamName4_), ParamName5_(ParamName5_), ParamName6_(ParamName6_), ParamName7_(ParamName7_), ParamName8_(ParamName8_), ParamName9_(ParamName9_), ParamName10_(ParamName10_) { } \
		} __scope_exit_args(ParamName1_, ParamName2_, ParamName3_, ParamName4_, ParamName5_, ParamName6_, ParamName7_, ParamName8_, ParamName9_, ParamName10_);



#define STINGRAY_SCOPE_EXIT(...) \
		TOOLKIT_CAT(STINGRAY_DECLARE_SCOPE_EXIT_ARGS_, TOOLKIT_NARGS(__VA_ARGS__))(__VA_ARGS__) \
		class TOOLKIT_CAT(__ScopeExitFunc, __LINE__) : protected TOOLKIT_CAT(__ScopeExitArgs, __LINE__) \
		{ \
		public: \
			TOOLKIT_CAT(__ScopeExitFunc, __LINE__) (TOOLKIT_CAT(__ScopeExitArgs, __LINE__) args) : TOOLKIT_CAT(__ScopeExitArgs, __LINE__)(args) { } \
			~TOOLKIT_CAT(__ScopeExitFunc, __LINE__) () {

#define STINGRAY_SCOPE_EXIT_END \
			} \
		} __scope_exit(__scope_exit_args);


}

/*! \endcond */


#endif
