#ifndef STINGRAY_TOOLKIT_ICREATOR_H
#define STINGRAY_TOOLKIT_ICREATOR_H


#include <stingray/toolkit/shared_ptr.h>
#include <stingray/toolkit/TypeList.h>


#define TOOLKIT_DECLARE_CREATOR(ClassName) \
		typedef stingray::ICreator<ClassName>				ClassName##Creator; \
		TOOLKIT_DECLARE_PTR(ClassName##Creator)

namespace stingray
{
	
	template < typename T >
	struct ICreator
	{
		virtual ~ICreator() { }

		virtual shared_ptr<T> Create() const = 0;
	};


#define DETAIL_DVRLIB_DECLARE_CONSTRUCTOR_CREATOR(N_, ParamTypesDecl_, ParamMembersDecl_, CtorParamsDecl_, CtorParamsInit_, CtorParams_) \
	template < typename InterfaceType, typename ClassType, ParamTypesDecl_> \
	class ConstructorCreator##N_ : public virtual ICreator<InterfaceType> \
	{ \
	private: \
		ParamMembersDecl_ \
	public: \
		ConstructorCreator##N_(CtorParamsDecl_) : CtorParamsInit_ { } \
		virtual shared_ptr<InterfaceType> Create() const { return make_shared<ClassType>(CtorParams_); } \
	}

#define PTD(I_) typename T##I_
#define PMD(I_) T##I_ _arg##I_
#define CPD(I_) const T##I_& arg##I_
#define CPI(I_) _arg##I_(arg##I_)
#define CP(I_) _arg##I_

	DETAIL_DVRLIB_DECLARE_CONSTRUCTOR_CREATOR(1, MK_PARAM(PTD(1)), MK_PARAM(PMD(1);), MK_PARAM(CPD(1)), MK_PARAM(CPI(1)), MK_PARAM(CP(1)));
	DETAIL_DVRLIB_DECLARE_CONSTRUCTOR_CREATOR(2, MK_PARAM(PTD(1), PTD(2)), MK_PARAM(PMD(1); PMD(2);), MK_PARAM(CPD(1), CPD(2)), MK_PARAM(CPI(1), CPI(2)), MK_PARAM(CP(1), CP(2)));
	DETAIL_DVRLIB_DECLARE_CONSTRUCTOR_CREATOR(3, MK_PARAM(PTD(1), PTD(2), PTD(3)), MK_PARAM(PMD(1); PMD(2); PMD(3);), MK_PARAM(CPD(1), CPD(2), CPD(3)), MK_PARAM(CPI(1), CPI(2), CPI(3)), MK_PARAM(CP(1), CP(2), CP(3)));

#undef CP
#undef CPI
#undef CPD
#undef PMD
#undef PTD

#undef DETAIL_DVRLIB_DECLARE_CONSTRUCTOR_CREATOR

	template < typename InterfaceType, typename ClassType >
	struct DefaultConstructorCreator : public virtual ICreator<InterfaceType>
	{
		virtual shared_ptr<InterfaceType> Create() const { return make_shared<ClassType>(); }
	};

	template <typename ClassType >
	struct SingleInstanceCreatorBase
	{
	protected:
		shared_ptr<ClassType> GetInstance() const
		{
			static shared_ptr<ClassType> instance(new ClassType);
			return instance;
		}
	};

	template < typename InterfaceType, typename ClassType >
	struct SingleInstanceCreator : 
		public virtual ICreator<InterfaceType>, private SingleInstanceCreatorBase<ClassType>	{
		virtual shared_ptr<InterfaceType> Create() const 
		{
			return this->GetInstance();
		}
	};


}


#endif
