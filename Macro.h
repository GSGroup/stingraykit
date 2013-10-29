#ifndef STINGRAY_TOOLKIT_MACRO_H
#define STINGRAY_TOOLKIT_MACRO_H

/**
 * @addtogroup toolkit_macro
 * @{
 */

#define MK_PARAM(...)	__VA_ARGS__

#define TO_STRING_IMPL(...) #__VA_ARGS__
#define TO_STRING(...) TO_STRING_IMPL(__VA_ARGS__)

#define TOOLKIT_CAT_IMPL(A, B)	A##B
#define TOOLKIT_CAT(A, B)	TOOLKIT_CAT_IMPL(A, B)

#define TOOLKIT_NARGS_SEQ(Dummy_, _1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,N,...) N
#define TOOLKIT_NARGS(...) TOOLKIT_NARGS_SEQ(Dummy_, ##__VA_ARGS__, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

#if 0
#define ______TOOLKIT_MACRO_FOR_0(FUNC)
#define ______TOOLKIT_MACRO_FOR_1(FUNC) FUNC(1)
#define ______TOOLKIT_MACRO_FOR_2(FUNC) TOOLKIT_MACRO_FOR_1(FUNC) FUNC(2)
#define ______TOOLKIT_MACRO_FOR_3(FUNC) TOOLKIT_MACRO_FOR_2(FUNC) FUNC(3)
#define ______TOOLKIT_MACRO_FOR_4(FUNC) TOOLKIT_MACRO_FOR_3(FUNC) FUNC(4)

#define ______TOOLKIT_MACRO_FOR(N, FUNC) TOOLKIT_MACRO_FOR_##N(FUNC)
#endif

/** @} */

#endif
