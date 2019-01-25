#ifndef STINGRAYKIT_LOCALVISITOR_H
#define STINGRAYKIT_LOCALVISITOR_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <stingraykit/metaprogramming/TypeList.h>
#include <stingraykit/static_visitor.h>


namespace stingray
{
	/**
	 * @addtogroup toolkit_general_variants
	 * @{
	 */

	/**
	 * @brief Start of the local visitor code
	 * @param[in] RetType_ The return type of the visitor
	 * @param[in] Variant_ The variant type
	 * @param[mixed] ... Each parameter is an MK_PARAM pair of the type and the name of a closure variable.
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
	 *	typedef variant< TypeList<OpFailed, OpSucceeded> >		OpResult;
	 *
	 *	class Class
	 *	{
	 *		ByteArray	_data;
	 *		IDialogPtr	_failDialog;
	 *		...
	 *
	 *		void OperationHandler(const OpResult& result)
	 *		{
	 *			STINGRAYKIT_LOCAL_VISITOR(std::string, OpResult, MK_PARAM(ByteArray&, _data), MK_PARAM(IDialogPtr, _failDialog))
	 *				STINGRAYKIT_LOCAL_VISITOR_CASE(OpFailed)
	 *					_failDialog->SetText("The operation failed, the error is: " + value.Error);
	 *					_failDialog->Show();
	 *					return "failed!";
	 *				STINGRAYKIT_LOCAL_VISITOR_CASE(OpSucceeded)
	 *					_data = value.Result;
	 *					return "succeded!";
	 *			STINGRAYKIT_LOCAL_VISITOR_END;
	 *
	 *			Logger::Info() << "The operation " << apply_visitor(local_visitor.GetVisitor(), v);
	 *		}
	 *	};
	 * @endcode
	 */
#define STINGRAYKIT_LOCAL_VISITOR(RetType_, Variant_, ...) \
		STINGRAYKIT_CAT(STINGRAYKIT_DECLARE_LOCAL_VISITOR_ARGS_, STINGRAYKIT_NARGS(__VA_ARGS__))(__VA_ARGS__) \
		class STINGRAYKIT_CAT(LocalVisitor, __LINE__) : protected stingray::LocalVisitorBase<RetType_, Variant_::TypeList>, protected STINGRAYKIT_CAT(LocalVisitorArgs, __LINE__) \
		{ \
			typedef LocalVisitorBase<RetType_, Variant_::TypeList>		base; \
		public: \
			const base& GetVisitor() const { return *this; } \
			STINGRAYKIT_CAT(LocalVisitor, __LINE__)(const STINGRAYKIT_CAT(LocalVisitorArgs, __LINE__)& args) : STINGRAYKIT_CAT(LocalVisitorArgs, __LINE__)(args) {

	/**
	 * @brief Start of the code that should be executed if the visited variant holds an object of type Type_
	 * @param[in] Type_ Type of the object in the visited variant
	 */
#define STINGRAYKIT_LOCAL_VISITOR_CASE(Type_) \
			} \
			virtual base::RetType Visit(const Type_& value) const {

	/** @brief End of the local visitor */
#define STINGRAYKIT_LOCAL_VISITOR_END \
			} \
		} local_visitor(__local_visitor_args);

	/** @} */


	template < typename RetType_, typename TypeList_, int Count_ = GetTypeListLength<TypeList_>::Value >
	class LocalVisitorBase;

	template < typename RetType_, typename TypeList_ >
	class LocalVisitorBase<RetType_, TypeList_, 1> : public static_visitor<RetType_>
	{
	public:
		virtual ~LocalVisitorBase() { }
		template < typename T > RetType_ operator () (const T& v) const { return Visit(v); }
		virtual RetType_ Visit(const typename GetTypeListItem<TypeList_, 0>::ValueT& value) const = 0;
	};

	template < typename RetType_, typename TypeList_ >
	class LocalVisitorBase<RetType_, TypeList_, 2> : public static_visitor<RetType_>
	{
	public:
		virtual ~LocalVisitorBase() { }
		template < typename T > RetType_ operator () (const T& v) const { return Visit(v); }
		virtual RetType_ Visit(const typename GetTypeListItem<TypeList_, 0>::ValueT& value) const = 0;
		virtual RetType_ Visit(const typename GetTypeListItem<TypeList_, 1>::ValueT& value) const = 0;
	};

	template < typename RetType_, typename TypeList_ >
	class LocalVisitorBase<RetType_, TypeList_, 3> : public static_visitor<RetType_>
	{
	public:
		virtual ~LocalVisitorBase() { }
		template < typename T > RetType_ operator () (const T& v) const { return Visit(v); }
		virtual RetType_ Visit(const typename GetTypeListItem<TypeList_, 0>::ValueT& value) const = 0;
		virtual RetType_ Visit(const typename GetTypeListItem<TypeList_, 1>::ValueT& value) const = 0;
		virtual RetType_ Visit(const typename GetTypeListItem<TypeList_, 2>::ValueT& value) const = 0;
	};

	template < typename RetType_, typename TypeList_ >
	class LocalVisitorBase<RetType_, TypeList_, 4> : public static_visitor<RetType_>
	{
	public:
		virtual ~LocalVisitorBase() { }
		template < typename T > RetType_ operator () (const T& v) const { return Visit(v); }
		virtual RetType_ Visit(const typename GetTypeListItem<TypeList_, 0>::ValueT& value) const = 0;
		virtual RetType_ Visit(const typename GetTypeListItem<TypeList_, 1>::ValueT& value) const = 0;
		virtual RetType_ Visit(const typename GetTypeListItem<TypeList_, 2>::ValueT& value) const = 0;
		virtual RetType_ Visit(const typename GetTypeListItem<TypeList_, 3>::ValueT& value) const = 0;
	};

	template < typename RetType_, typename TypeList_ >
	class LocalVisitorBase<RetType_, TypeList_, 5> : public static_visitor<RetType_>
	{
	public:
		virtual ~LocalVisitorBase() { }
		template < typename T > RetType_ operator () (const T& v) const { return Visit(v); }
		virtual RetType_ Visit(const typename GetTypeListItem<TypeList_, 0>::ValueT& value) const = 0;
		virtual RetType_ Visit(const typename GetTypeListItem<TypeList_, 1>::ValueT& value) const = 0;
		virtual RetType_ Visit(const typename GetTypeListItem<TypeList_, 2>::ValueT& value) const = 0;
		virtual RetType_ Visit(const typename GetTypeListItem<TypeList_, 3>::ValueT& value) const = 0;
		virtual RetType_ Visit(const typename GetTypeListItem<TypeList_, 4>::ValueT& value) const = 0;
	};

	template < typename RetType_, typename TypeList_ >
	class LocalVisitorBase<RetType_, TypeList_, 6> : public static_visitor<RetType_>
	{
	public:
		virtual ~LocalVisitorBase() { }
		template < typename T > RetType_ operator () (const T& v) const { return Visit(v); }
		virtual RetType_ Visit(const typename GetTypeListItem<TypeList_, 0>::ValueT& value) const = 0;
		virtual RetType_ Visit(const typename GetTypeListItem<TypeList_, 1>::ValueT& value) const = 0;
		virtual RetType_ Visit(const typename GetTypeListItem<TypeList_, 2>::ValueT& value) const = 0;
		virtual RetType_ Visit(const typename GetTypeListItem<TypeList_, 3>::ValueT& value) const = 0;
		virtual RetType_ Visit(const typename GetTypeListItem<TypeList_, 4>::ValueT& value) const = 0;
		virtual RetType_ Visit(const typename GetTypeListItem<TypeList_, 5>::ValueT& value) const = 0;
	};

	template < typename RetType_, typename TypeList_ >
	class LocalVisitorBase<RetType_, TypeList_, 7> : public static_visitor<RetType_>
	{
	public:
		virtual ~LocalVisitorBase() { }
		template < typename T > RetType_ operator () (const T& v) const { return Visit(v); }
		virtual RetType_ Visit(const typename GetTypeListItem<TypeList_, 0>::ValueT& value) const = 0;
		virtual RetType_ Visit(const typename GetTypeListItem<TypeList_, 1>::ValueT& value) const = 0;
		virtual RetType_ Visit(const typename GetTypeListItem<TypeList_, 2>::ValueT& value) const = 0;
		virtual RetType_ Visit(const typename GetTypeListItem<TypeList_, 3>::ValueT& value) const = 0;
		virtual RetType_ Visit(const typename GetTypeListItem<TypeList_, 4>::ValueT& value) const = 0;
		virtual RetType_ Visit(const typename GetTypeListItem<TypeList_, 5>::ValueT& value) const = 0;
		virtual RetType_ Visit(const typename GetTypeListItem<TypeList_, 6>::ValueT& value) const = 0;
	};

	template < typename RetType_, typename TypeList_ >
	class LocalVisitorBase<RetType_, TypeList_, 8> : public static_visitor<RetType_>
	{
	public:
		virtual ~LocalVisitorBase() { }
		template < typename T > RetType_ operator () (const T& v) const { return Visit(v); }
		virtual RetType_ Visit(const typename GetTypeListItem<TypeList_, 0>::ValueT& value) const = 0;
		virtual RetType_ Visit(const typename GetTypeListItem<TypeList_, 1>::ValueT& value) const = 0;
		virtual RetType_ Visit(const typename GetTypeListItem<TypeList_, 2>::ValueT& value) const = 0;
		virtual RetType_ Visit(const typename GetTypeListItem<TypeList_, 3>::ValueT& value) const = 0;
		virtual RetType_ Visit(const typename GetTypeListItem<TypeList_, 4>::ValueT& value) const = 0;
		virtual RetType_ Visit(const typename GetTypeListItem<TypeList_, 5>::ValueT& value) const = 0;
		virtual RetType_ Visit(const typename GetTypeListItem<TypeList_, 6>::ValueT& value) const = 0;
		virtual RetType_ Visit(const typename GetTypeListItem<TypeList_, 7>::ValueT& value) const = 0;
	};

	template < typename RetType_, typename TypeList_ >
	class LocalVisitorBase<RetType_, TypeList_, 9> : public static_visitor<RetType_>
	{
	public:
		virtual ~LocalVisitorBase() { }
		template < typename T > RetType_ operator () (const T& v) const { return Visit(v); }
		virtual RetType_ Visit(const typename GetTypeListItem<TypeList_, 0>::ValueT& value) const = 0;
		virtual RetType_ Visit(const typename GetTypeListItem<TypeList_, 1>::ValueT& value) const = 0;
		virtual RetType_ Visit(const typename GetTypeListItem<TypeList_, 2>::ValueT& value) const = 0;
		virtual RetType_ Visit(const typename GetTypeListItem<TypeList_, 3>::ValueT& value) const = 0;
		virtual RetType_ Visit(const typename GetTypeListItem<TypeList_, 4>::ValueT& value) const = 0;
		virtual RetType_ Visit(const typename GetTypeListItem<TypeList_, 5>::ValueT& value) const = 0;
		virtual RetType_ Visit(const typename GetTypeListItem<TypeList_, 6>::ValueT& value) const = 0;
		virtual RetType_ Visit(const typename GetTypeListItem<TypeList_, 7>::ValueT& value) const = 0;
		virtual RetType_ Visit(const typename GetTypeListItem<TypeList_, 8>::ValueT& value) const = 0;
	};

	template < typename RetType_, typename TypeList_ >
	class LocalVisitorBase<RetType_, TypeList_, 10> : public static_visitor<RetType_>
	{
	public:
		virtual ~LocalVisitorBase() { }
		template < typename T > RetType_ operator () (const T& v) const { return Visit(v); }
		virtual RetType_ Visit(const typename GetTypeListItem<TypeList_, 0>::ValueT& value) const = 0;
		virtual RetType_ Visit(const typename GetTypeListItem<TypeList_, 1>::ValueT& value) const = 0;
		virtual RetType_ Visit(const typename GetTypeListItem<TypeList_, 2>::ValueT& value) const = 0;
		virtual RetType_ Visit(const typename GetTypeListItem<TypeList_, 3>::ValueT& value) const = 0;
		virtual RetType_ Visit(const typename GetTypeListItem<TypeList_, 4>::ValueT& value) const = 0;
		virtual RetType_ Visit(const typename GetTypeListItem<TypeList_, 5>::ValueT& value) const = 0;
		virtual RetType_ Visit(const typename GetTypeListItem<TypeList_, 6>::ValueT& value) const = 0;
		virtual RetType_ Visit(const typename GetTypeListItem<TypeList_, 7>::ValueT& value) const = 0;
		virtual RetType_ Visit(const typename GetTypeListItem<TypeList_, 8>::ValueT& value) const = 0;
		virtual RetType_ Visit(const typename GetTypeListItem<TypeList_, 9>::ValueT& value) const = 0;
	};


#define STINGRAYKIT_DECLARE_LOCAL_VISITOR_ARGS_0() \
		class STINGRAYKIT_CAT(LocalVisitorArgs, __LINE__) { \
		protected: \
		public: \
			STINGRAYKIT_CAT(LocalVisitorArgs, __LINE__)() \
			{ } \
		} __local_visitor_args; \


#define STINGRAYKIT_DECLARE_LOCAL_VISITOR_ARGS_2(ParamType1_, ParamName1_) \
		class STINGRAYKIT_CAT(LocalVisitorArgs, __LINE__) { \
		protected: \
			ParamType1_		ParamName1_; \
		public: \
			STINGRAYKIT_CAT(LocalVisitorArgs, __LINE__)(ParamType1_ ParamName1_) \
				: ParamName1_(ParamName1_) \
			{ } \
		} __local_visitor_args(ParamName1_); \


#define STINGRAYKIT_DECLARE_LOCAL_VISITOR_ARGS_4(ParamType1_, ParamName1_, ParamType2_, ParamName2_) \
		class STINGRAYKIT_CAT(LocalVisitorArgs, __LINE__) { \
		protected: \
			ParamType1_		ParamName1_; \
			ParamType2_		ParamName2_; \
		public: \
			STINGRAYKIT_CAT(LocalVisitorArgs, __LINE__)(ParamType1_ ParamName1_, ParamType2_ ParamName2_) \
				: ParamName1_(ParamName1_), ParamName2_(ParamName2_) \
			{ } \
		} __local_visitor_args(ParamName1_, ParamName2_); \

#define STINGRAYKIT_DECLARE_LOCAL_VISITOR_ARGS_6(ParamType1_, ParamName1_, ParamType2_, ParamName2_, ParamType3_, ParamName3_) \
		class STINGRAYKIT_CAT(LocalVisitorArgs, __LINE__) { \
		protected: \
			ParamType1_		ParamName1_; \
			ParamType2_		ParamName2_; \
			ParamType3_		ParamName3_; \
		public: \
			STINGRAYKIT_CAT(LocalVisitorArgs, __LINE__)(ParamType1_ ParamName1_, ParamType2_ ParamName2_, ParamType3_ ParamName3_) \
				: ParamName1_(ParamName1_), ParamName2_(ParamName2_), ParamName3_(ParamName3_) \
			{ } \
		} __local_visitor_args(ParamName1_, ParamName2_, ParamName3_); \

#define STINGRAYKIT_DECLARE_LOCAL_VISITOR_ARGS_8(ParamType1_, ParamName1_, ParamType2_, ParamName2_, ParamType3_, ParamName3_, ParamType4_, ParamName4_) \
		class STINGRAYKIT_CAT(LocalVisitorArgs, __LINE__) { \
		protected: \
			ParamType1_		ParamName1_; \
			ParamType2_		ParamName2_; \
			ParamType3_		ParamName3_; \
			ParamType4_		ParamName4_; \
		public: \
			STINGRAYKIT_CAT(LocalVisitorArgs, __LINE__)(ParamType1_ ParamName1_, ParamType2_ ParamName2_, ParamType3_ ParamName3_, ParamType4_ ParamName4_) \
				: ParamName1_(ParamName1_), ParamName2_(ParamName2_), ParamName3_(ParamName3_), ParamName4_(ParamName4_) \
			{ } \
		} __local_visitor_args(ParamName1_, ParamName2_, ParamName3_, ParamName4_); \

#define STINGRAYKIT_DECLARE_LOCAL_VISITOR_ARGS_10(ParamType1_, ParamName1_, ParamType2_, ParamName2_, ParamType3_, ParamName3_, ParamType4_, ParamName4_, ParamType5_, ParamName5_) \
		class STINGRAYKIT_CAT(LocalVisitorArgs, __LINE__) { \
		protected: \
			ParamType1_		ParamName1_; \
			ParamType2_		ParamName2_; \
			ParamType3_		ParamName3_; \
			ParamType4_		ParamName4_; \
			ParamType5_		ParamName5_; \
		public: \
			STINGRAYKIT_CAT(LocalVisitorArgs, __LINE__)(ParamType1_ ParamName1_, ParamType2_ ParamName2_, ParamType3_ ParamName3_, ParamType4_ ParamName4_, ParamType5_ ParamName5_) \
				: ParamName1_(ParamName1_), ParamName2_(ParamName2_), ParamName3_(ParamName3_), ParamName4_(ParamName4_), ParamName5_(ParamName5_) \
			{ } \
		} __local_visitor_args(ParamName1_, ParamName2_, ParamName3_, ParamName4_, ParamName5_); \

#define STINGRAYKIT_DECLARE_LOCAL_VISITOR_ARGS_12(ParamType1_, ParamName1_, ParamType2_, ParamName2_, ParamType3_, ParamName3_, ParamType4_, ParamName4_, ParamType5_, ParamName5_, ParamType6_, ParamName6_) \
		class STINGRAYKIT_CAT(LocalVisitorArgs, __LINE__) { \
		protected: \
			ParamType1_		ParamName1_; \
			ParamType2_		ParamName2_; \
			ParamType3_		ParamName3_; \
			ParamType4_		ParamName4_; \
			ParamType5_		ParamName5_; \
			ParamType6_		ParamName6_; \
		public: \
			STINGRAYKIT_CAT(LocalVisitorArgs, __LINE__)(ParamType1_ ParamName1_, ParamType2_ ParamName2_, ParamType3_ ParamName3_, ParamType4_ ParamName4_, ParamType5_ ParamName5_, ParamType6_ ParamName6_) \
				: ParamName1_(ParamName1_), ParamName2_(ParamName2_), ParamName3_(ParamName3_), ParamName4_(ParamName4_), ParamName5_(ParamName5_), ParamName6_(ParamName6_) \
			{ } \
		} __local_visitor_args(ParamName1_, ParamName2_, ParamName3_, ParamName4_, ParamName5_, ParamName6_); \

#define STINGRAYKIT_DECLARE_LOCAL_VISITOR_ARGS_14(ParamType1_, ParamName1_, ParamType2_, ParamName2_, ParamType3_, ParamName3_, ParamType4_, ParamName4_, ParamType5_, ParamName5_, ParamType6_, ParamName6_, ParamType7_, ParamName7_) \
		class STINGRAYKIT_CAT(LocalVisitorArgs, __LINE__) { \
		protected: \
			ParamType1_		ParamName1_; \
			ParamType2_		ParamName2_; \
			ParamType3_		ParamName3_; \
			ParamType4_		ParamName4_; \
			ParamType5_		ParamName5_; \
			ParamType6_		ParamName6_; \
			ParamType7_		ParamName7_; \
		public: \
			STINGRAYKIT_CAT(LocalVisitorArgs, __LINE__)(ParamType1_ ParamName1_, ParamType2_ ParamName2_, ParamType3_ ParamName3_, ParamType4_ ParamName4_, ParamType5_ ParamName5_, ParamType6_ ParamName6_, ParamType7_ ParamName7_) \
				: ParamName1_(ParamName1_), ParamName2_(ParamName2_), ParamName3_(ParamName3_), ParamName4_(ParamName4_), ParamName5_(ParamName5_), ParamName6_(ParamName6_), ParamName7_(ParamName7_) \
			{ } \
		} __local_visitor_args(ParamName1_, ParamName2_, ParamName3_, ParamName4_, ParamName5_, ParamName6_, ParamName7_); \

#define STINGRAYKIT_DECLARE_LOCAL_VISITOR_ARGS_16(ParamType1_, ParamName1_, ParamType2_, ParamName2_, ParamType3_, ParamName3_, ParamType4_, ParamName4_, ParamType5_, ParamName5_, ParamType6_, ParamName6_, ParamType7_, ParamName7_, ParamType8_, ParamName8_) \
		class STINGRAYKIT_CAT(LocalVisitorArgs, __LINE__) { \
		protected: \
			ParamType1_		ParamName1_; \
			ParamType2_		ParamName2_; \
			ParamType3_		ParamName3_; \
			ParamType4_		ParamName4_; \
			ParamType5_		ParamName5_; \
			ParamType6_		ParamName6_; \
			ParamType7_		ParamName7_; \
			ParamType8_		ParamName8_; \
		public: \
			STINGRAYKIT_CAT(LocalVisitorArgs, __LINE__)(ParamType1_ ParamName1_, ParamType2_ ParamName2_, ParamType3_ ParamName3_, ParamType4_ ParamName4_, ParamType5_ ParamName5_, ParamType6_ ParamName6_, ParamType7_ ParamName7_, ParamType8_ ParamName8_) \
				: ParamName1_(ParamName1_), ParamName2_(ParamName2_), ParamName3_(ParamName3_), ParamName4_(ParamName4_), ParamName5_(ParamName5_), ParamName6_(ParamName6_), ParamName7_(ParamName7_), ParamName8_(ParamName8_) \
			{ } \
		} __local_visitor_args(ParamName1_, ParamName2_, ParamName3_, ParamName4_, ParamName5_, ParamName6_, ParamName7_, ParamName8_); \

#define STINGRAYKIT_DECLARE_LOCAL_VISITOR_ARGS_18(ParamType1_, ParamName1_, ParamType2_, ParamName2_, ParamType3_, ParamName3_, ParamType4_, ParamName4_, ParamType5_, ParamName5_, ParamType6_, ParamName6_, ParamType7_, ParamName7_, ParamType8_, ParamName8_, ParamType9_, ParamName9_) \
		class STINGRAYKIT_CAT(LocalVisitorArgs, __LINE__) { \
		protected: \
			ParamType1_		ParamName1_; \
			ParamType2_		ParamName2_; \
			ParamType3_		ParamName3_; \
			ParamType4_		ParamName4_; \
			ParamType5_		ParamName5_; \
			ParamType6_		ParamName6_; \
			ParamType7_		ParamName7_; \
			ParamType8_		ParamName8_; \
			ParamType9_		ParamName9_; \
		public: \
			STINGRAYKIT_CAT(LocalVisitorArgs, __LINE__)(ParamType1_ ParamName1_, ParamType2_ ParamName2_, ParamType3_ ParamName3_, ParamType4_ ParamName4_, ParamType5_ ParamName5_, ParamType6_ ParamName6_, ParamType7_ ParamName7_, ParamType8_ ParamName8_, ParamType9_ ParamName9_) \
				: ParamName1_(ParamName1_), ParamName2_(ParamName2_), ParamName3_(ParamName3_), ParamName4_(ParamName4_), ParamName5_(ParamName5_), ParamName6_(ParamName6_), ParamName7_(ParamName7_), ParamName8_(ParamName8_), ParamName9_(ParamName9_) \
			{ } \
		} __local_visitor_args(ParamName1_, ParamName2_, ParamName3_, ParamName4_, ParamName5_, ParamName6_, ParamName7_, ParamName8_, ParamName9_); \

#define STINGRAYKIT_DECLARE_LOCAL_VISITOR_ARGS_20(ParamType1_, ParamName1_, ParamType2_, ParamName2_, ParamType3_, ParamName3_, ParamType4_, ParamName4_, ParamType5_, ParamName5_, ParamType6_, ParamName6_, ParamType7_, ParamName7_, ParamType8_, ParamName8_, ParamType9_, ParamName9_, ParamType10_, ParamName10_) \
		class STINGRAYKIT_CAT(LocalVisitorArgs, __LINE__) { \
		protected: \
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
		public: \
			STINGRAYKIT_CAT(LocalVisitorArgs, __LINE__)(ParamType1_ ParamName1_, ParamType2_ ParamName2_, ParamType3_ ParamName3_, ParamType4_ ParamName4_, ParamType5_ ParamName5_, ParamType6_ ParamName6_, ParamType7_ ParamName7_, ParamType8_ ParamName8_, ParamType9_ ParamName9_, ParamType10_ ParamName10_) \
				: ParamName1_(ParamName1_), ParamName2_(ParamName2_), ParamName3_(ParamName3_), ParamName4_(ParamName4_), ParamName5_(ParamName5_), ParamName6_(ParamName6_), ParamName7_(ParamName7_), ParamName8_(ParamName8_), ParamName9_(ParamName9_), ParamName10_(ParamName10_) \
			{ } \
		} __local_visitor_args(ParamName1_, ParamName2_, ParamName3_, ParamName4_, ParamName5_, ParamName6_, ParamName7_, ParamName8_, ParamName9_, ParamName10_); \

}

#endif
