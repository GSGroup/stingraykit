#ifndef STINGRAY_TOOLKIT_FUNCTION_H
#define STINGRAY_TOOLKIT_FUNCTION_H

#include <stingray/toolkit/FunctorInvoker.h>
#include <stingray/toolkit/Tuple.h>
#include <stingray/toolkit/function_info.h>
#include <stingray/toolkit/function_name_getter.h>
#include <stingray/toolkit/self_counter.h>

/*! \cond GS_INTERNAL */

namespace stingray
{

	template < typename Signature >
	class function;


	namespace Detail
	{
		TOOLKIT_DECLARE_NESTED_TYPE_CHECK(RetType);
		TOOLKIT_DECLARE_NESTED_TYPE_CHECK(ParamTypes);

		template<typename FunctorType, bool B = function_type<FunctorType>::Type == FunctionType::Other>
		struct FunctorTypeValidator
		{ static const bool Value = HasNestedType_RetType<FunctorType>::Value && HasNestedType_ParamTypes<FunctorType>::Value; };

		template<typename FunctorType>
		struct FunctorTypeValidator<FunctorType, false>
		{ static const bool Value = true; };

		template
			<
				typename Signature, typename FunctorType,
				bool HasMetaInfo = FunctorTypeValidator<FunctorType>::Value
			>
		struct FunctorTypeTransformer
		{ typedef FunctorType ValueT; };

		template < typename Signature, typename FunctorType >
		struct FunctorTypeTransformer<Signature, FunctorType, false>
		{
			struct ValueT : public FunctorType, public function_info<Signature>
			{
				ValueT(const FunctorType& baseObj)
					: FunctorType(baseObj)
				{ }
			};
		};

		struct IInvokableBase : public self_counter<IInvokableBase>
		{
			struct VTable
			{
				typedef void DtorFunc(IInvokableBase *self);
				typedef void InvokeFunc(void);
				typedef std::string GetNameFunc(const IInvokableBase *self);

				typedef InvokeFunc *	InvokePtr;
				typedef DtorFunc *		DtorFuncPtr;
				typedef GetNameFunc *	GetNameFuncPtr;

				DtorFuncPtr		Dtor;
				InvokePtr		Invoke;
				GetNameFuncPtr	GetName;

				VTable(DtorFuncPtr dtor, InvokePtr invoke, GetNameFuncPtr getName) : Dtor(dtor), Invoke(invoke), GetName(getName) { }
			};
			typedef VTable GetVTableFunc();
			GetVTableFunc	*_getVTable;

			inline IInvokableBase(GetVTableFunc* func) : _getVTable(func) {}
			inline ~IInvokableBase() { _getVTable().Dtor(this); }
		};

		template < typename Signature_ >
		struct IInvokable : public IInvokableBase
		{
			typedef IInvokableBase									base;
			typedef Signature_										Signature;
			typedef typename function_info<Signature>::RetType		RetType;
			typedef typename function_info<Signature>::ParamTypes	ParamTypes;

			typedef RetType InvokeFunc(IInvokable *self, const Tuple<ParamTypes>& p);

			inline IInvokable(GetVTableFunc* func) : base(func) {}
		};


		template < typename Signature, typename FunctorType, bool HasReturnType = !SameType<typename function_info<Signature>::RetType, void>::Value >
		class Invokable : public IInvokable<Signature>
		{
			TOOLKIT_NONCOPYABLE(Invokable);

			typedef IInvokable<Signature>		BaseType;
			typedef typename BaseType::VTable	VTable;
			typedef Invokable<Signature, FunctorType, HasReturnType> MyType;

		public:
			typedef typename BaseType::InvokeFunc InvokeFunc;

		private:
			FunctorType	_func;
			typedef typename FunctorTypeTransformer<Signature, FunctorType>::ValueT WrappedFunctorType;

		public:
			inline Invokable(const FunctorType& func)
				: BaseType(&MyType::GetVTable), _func(func)
			{}

			static inline VTable GetVTable()
			{ return VTable(&MyType::Dtor, reinterpret_cast<typename VTable::InvokePtr>(&MyType::Invoke), &MyType::GetName); }

		protected:
			inline typename BaseType::RetType DoInvoke(const Tuple<typename BaseType::ParamTypes>& p)
			{ return FunctorInvoker::Invoke<WrappedFunctorType>(_func, p); }

			static inline typename BaseType::RetType Invoke(BaseType *self, const Tuple<typename BaseType::ParamTypes>& p)
			{ return static_cast<MyType *>(self)->DoInvoke(p); }

			static inline void Dtor(IInvokableBase *self)
			{ static_cast<MyType *>(self)->_func.~FunctorType(); }

			static std::string GetName(const IInvokableBase *self)
			{ return get_function_name(static_cast<const MyType *>(self)->_func); }
		};

		template < typename Signature, typename FunctorType >
		class Invokable<Signature, FunctorType, false> : public IInvokable<Signature>
		{
			TOOLKIT_NONCOPYABLE(Invokable);

			typedef IInvokable<Signature>		BaseType;
			typedef typename BaseType::VTable	VTable;
			typedef Invokable<Signature, FunctorType, false> MyType;

		public:
			typedef typename BaseType::InvokeFunc InvokeFunc;

		private:
			FunctorType	_func;
			typedef typename FunctorTypeTransformer<Signature, FunctorType>::ValueT WrappedFunctorType;

		public:
			inline Invokable(const FunctorType& func)
				: BaseType(&MyType::GetVTable), _func(func)
			{}

			static inline VTable GetVTable()
			{ return VTable(&MyType::Dtor, reinterpret_cast<typename VTable::InvokePtr>(&MyType::Invoke), &MyType::GetName); }

		protected:
			inline void DoInvoke(const Tuple<typename BaseType::ParamTypes>& p)
			{ FunctorInvoker::Invoke<WrappedFunctorType>(_func, p); }

			static inline void Invoke(BaseType *self, const Tuple<typename BaseType::ParamTypes>& p)
			{ static_cast<MyType *>(self)->DoInvoke(p); }

			static inline void Dtor(IInvokableBase *self)
			{ static_cast<MyType *>(self)->_func.~FunctorType(); }

			static std::string GetName(const IInvokableBase *self)
			{ return get_function_name(static_cast<const MyType *>(self)->_func); }
		};

	}

	class function_storage;

	template < typename Signature >
	class function_base : public function_info<Signature>
	{
	public:
		typedef typename function_info<Signature>::RetType		RetType;
		typedef typename function_info<Signature>::ParamTypes	ParamTypes;

	protected:
		typedef Detail::IInvokable<Signature>					InvokableType;
		typedef typename InvokableType::InvokeFunc				InvokeFunc;

		self_count_ptr<InvokableType>	_invokable;

	protected:
		inline ~function_base() {}
		template < typename FunctorType >
		inline function_base(const FunctorType& func)
			: _invokable(new Detail::Invokable<Signature, FunctorType>(func))
		{ }

		inline RetType Invoke(const Tuple<ParamTypes>& p) const
		{
			InvokeFunc *func = reinterpret_cast<InvokeFunc*>(_invokable->_getVTable().Invoke);
			return func(_invokable.get(), p);
		}

	public:
		std::string get_name() const { return _invokable->_getVTable().GetName(_invokable.get()); }

	private:
		friend class function_storage;
		explicit function_base(const self_count_ptr<InvokableType>& ptr) : _invokable(ptr)
		{}
	};

	template < typename RetType, typename ParamTypes, size_t ParamCount = GetTypeListLength<ParamTypes>::Value >
	struct FunctionConstructor;

	template < typename R >
	class function<R()> : public function_base<R()>
	{
		typedef function_base<R()> BaseType;

	public:
		template < typename FunctorType >
		inline function(const FunctorType& func)
			: function_base<R()>(func)
		{ }

		inline R operator ()() const
		{
			Tuple<TypeList_0> p;
			return this->Invoke(p);
		}

	private:
		friend class function_storage;
		explicit function(const BaseType& base) : BaseType(base)
		{}
	};

	template < typename RetType, typename ParamTypes >
	struct FunctionConstructor<RetType, ParamTypes, 0>
	{ typedef function<RetType()> 	ValueT; };

#define TY typename
#define PT(N_) typename GetTypeListItem<ParamTypes, N_ - 1>::ValueT

#define DETAIL_TOOLKIT_DECLARE_FUNCTION(ParamsCount_, ParamTypenames_, ParamTypes_, ParamDecl_, ParamUsage_, ParamsFromTypeList_) \
	template < typename R, ParamTypenames_ > \
	class function<R(ParamTypes_)> : public function_base<R(ParamTypes_)> \
	{ \
		typedef function_base<R(ParamTypes_)> BaseType; \
	public: \
		template < typename FunctorType > \
		function(const FunctorType& func) \
			: function_base<R(ParamTypes_)>(func) \
		{ } \
		\
		inline R operator ()(ParamDecl_) const \
		{  \
			Tuple<TYPELIST(ParamTypes_)> p(ParamUsage_); \
			return this->Invoke(p); \
		} \
	private: \
		friend class function_storage; \
		explicit function(const BaseType& base) : BaseType(base) \
		{} \
	}; \
	template < typename RetType, typename ParamTypes > \
	struct FunctionConstructor<RetType, ParamTypes, ParamsCount_ > \
	{ typedef function<RetType(ParamsFromTypeList_)>	ValueT; }

	//Please do not set inline to inline above, this will cause recursive force-inlining, which is incompatible with some compilers

	DETAIL_TOOLKIT_DECLARE_FUNCTION(1, MK_PARAM(TY T1), MK_PARAM(T1), MK_PARAM(T1 p1), MK_PARAM(p1), MK_PARAM(PT(1)));
	DETAIL_TOOLKIT_DECLARE_FUNCTION(2, MK_PARAM(TY T1, TY T2), MK_PARAM(T1, T2), MK_PARAM(T1 p1, T2 p2), MK_PARAM(p1, p2), MK_PARAM(PT(1), PT(2)));
	DETAIL_TOOLKIT_DECLARE_FUNCTION(3, MK_PARAM(TY T1, TY T2, TY T3), MK_PARAM(T1, T2, T3), MK_PARAM(T1 p1, T2 p2, T3 p3), MK_PARAM(p1, p2, p3), MK_PARAM(PT(1), PT(2), PT(3)));
	DETAIL_TOOLKIT_DECLARE_FUNCTION(4, MK_PARAM(TY T1, TY T2, TY T3, TY T4), MK_PARAM(T1, T2, T3, T4), MK_PARAM(T1 p1, T2 p2, T3 p3, T4 p4), MK_PARAM(p1, p2, p3, p4), MK_PARAM(PT(1), PT(2), PT(3), PT(4)));
	DETAIL_TOOLKIT_DECLARE_FUNCTION(5, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5), MK_PARAM(T1, T2, T3, T4, T5), MK_PARAM(T1 p1, T2 p2, T3 p3, T4 p4, T5 p5), MK_PARAM(p1, p2, p3, p4, p5), MK_PARAM(PT(1), PT(2), PT(3), PT(4), PT(5)));
	DETAIL_TOOLKIT_DECLARE_FUNCTION(6, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6), MK_PARAM(T1, T2, T3, T4, T5, T6), MK_PARAM(T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6), MK_PARAM(p1, p2, p3, p4, p5, p6), MK_PARAM(PT(1), PT(2), PT(3), PT(4), PT(5), PT(6)));
	DETAIL_TOOLKIT_DECLARE_FUNCTION(7, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7), MK_PARAM(T1, T2, T3, T4, T5, T6, T7), MK_PARAM(T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6, T7 p7), MK_PARAM(p1, p2, p3, p4, p5, p6, p7), MK_PARAM(PT(1), PT(2), PT(3), PT(4), PT(5), PT(6), PT(7)));
	DETAIL_TOOLKIT_DECLARE_FUNCTION(8, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8), MK_PARAM(T1, T2, T3, T4, T5, T6, T7, T8), MK_PARAM(T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6, T7 p7, T8 p8), MK_PARAM(p1, p2, p3, p4, p5, p6, p7, p8), MK_PARAM(PT(1), PT(2), PT(3), PT(4), PT(5), PT(6), PT(7), PT(8)));
	DETAIL_TOOLKIT_DECLARE_FUNCTION(9, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9), MK_PARAM(T1, T2, T3, T4, T5, T6, T7, T8, T9), MK_PARAM(T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6, T7 p7, T8 p8, T9 p9), MK_PARAM(p1, p2, p3, p4, p5, p6, p7, p8, p9), MK_PARAM(PT(1), PT(2), PT(3), PT(4), PT(5), PT(6), PT(7), PT(8), PT(9)));
	DETAIL_TOOLKIT_DECLARE_FUNCTION(10, MK_PARAM(TY T1, TY T2, TY T3, TY T4, TY T5, TY T6, TY T7, TY T8, TY T9, TY T10), MK_PARAM(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10), MK_PARAM(T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6, T7 p7, T8 p8, T9 p9, T10 p10), MK_PARAM(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10), MK_PARAM(PT(1), PT(2), PT(3), PT(4), PT(5), PT(6), PT(7), PT(8), PT(9), PT(10)));

#undef TY

	class function_storage
	{
	private:
		self_count_ptr<Detail::IInvokableBase>	_invokable;

	public:
		template<typename Signature>
		explicit function_storage(const function<Signature> &func) : _invokable(func._invokable, static_cast_tag())
		{ }

		template<typename Signature>
		function<Signature> ToFunction() const
		{
			typedef typename function_base<Signature>::InvokableType TargetInvokable;
			self_count_ptr<TargetInvokable> ptr(_invokable, static_cast_tag());
			return function<Signature>(function_base<Signature>(ptr));
		}
	};


}

/*! \endcond */


#endif
