#ifndef STINGRAY_TOOLKIT_ICREATOR_H
#define STINGRAY_TOOLKIT_ICREATOR_H


#include <stingray/toolkit/shared_ptr.h>


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


#define DETAIL_STINGRAY_DECLARE_CONSTRUCTOR_CREATOR(N_, ParamTypesDecl_, ParamTypes_, ParamMembersDecl_, CtorParamsDecl_, CtorParamsInit_, CtorParams_) \
	template < typename InterfaceType, typename ClassType, ParamTypesDecl_> \
	class ConstructorCreator##N_ : public virtual ICreator<InterfaceType> \
	{ \
	private: \
		ParamMembersDecl_ \
	public: \
		ConstructorCreator##N_(CtorParamsDecl_) : CtorParamsInit_ { } \
		virtual shared_ptr<InterfaceType> Create() const { return make_shared<ClassType>(CtorParams_); } \
	}; \
	template < typename InterfaceType, typename ClassType, ParamTypesDecl_ > \
	shared_ptr<ConstructorCreator##N_<InterfaceType, ClassType, ParamTypes_> > MakeConstructorCreator(CtorParamsDecl_) \
	{ return make_shared<ConstructorCreator##N_<InterfaceType, ClassType, ParamTypes_> >(CtorParams_); }

#define PTD(I_) typename T##I_
#define PT(I_) T##I_
#define PMD(I_) T##I_ arg##I_
#define CPD(I_) const T##I_& arg##I_
#define CPI(I_) arg##I_(arg##I_)
#define CP(I_) arg##I_

	DETAIL_STINGRAY_DECLARE_CONSTRUCTOR_CREATOR(1, MK_PARAM(PTD(1)), MK_PARAM(PT(1)), MK_PARAM(PMD(1);), MK_PARAM(CPD(1)), MK_PARAM(CPI(1)), MK_PARAM(CP(1)));
	DETAIL_STINGRAY_DECLARE_CONSTRUCTOR_CREATOR(2, MK_PARAM(PTD(1), PTD(2)), MK_PARAM(PT(1), PT(2)), MK_PARAM(PMD(1); PMD(2);), MK_PARAM(CPD(1), CPD(2)), MK_PARAM(CPI(1), CPI(2)), MK_PARAM(CP(1), CP(2)));
	DETAIL_STINGRAY_DECLARE_CONSTRUCTOR_CREATOR(3, MK_PARAM(PTD(1), PTD(2), PTD(3)), MK_PARAM(PT(1), PT(2), PT(3)), MK_PARAM(PMD(1); PMD(2); PMD(3);), MK_PARAM(CPD(1), CPD(2), CPD(3)), MK_PARAM(CPI(1), CPI(2), CPI(3)), MK_PARAM(CP(1), CP(2), CP(3)));
	DETAIL_STINGRAY_DECLARE_CONSTRUCTOR_CREATOR(4, MK_PARAM(PTD(1), PTD(2), PTD(3), PTD(4)), MK_PARAM(PT(1), PT(2), PT(3), PT(4)), MK_PARAM(PMD(1); PMD(2); PMD(3); PMD(4);), MK_PARAM(CPD(1), CPD(2), CPD(3), CPD(4)), MK_PARAM(CPI(1), CPI(2), CPI(3), CPI(4)), MK_PARAM(CP(1), CP(2), CP(3), CP(4)));
	DETAIL_STINGRAY_DECLARE_CONSTRUCTOR_CREATOR(5, MK_PARAM(PTD(1), PTD(2), PTD(3), PTD(4), PTD(5)), MK_PARAM(PT(1), PT(2), PT(3), PT(4), PT(5)), MK_PARAM(PMD(1); PMD(2); PMD(3); PMD(4); PMD(5);), MK_PARAM(CPD(1), CPD(2), CPD(3), CPD(4), CPD(5)), MK_PARAM(CPI(1), CPI(2), CPI(3), CPI(4), CPI(5)), MK_PARAM(CP(1), CP(2), CP(3), CP(4), CP(5)));

#undef CP
#undef CPI
#undef CPD
#undef PMD
#undef PT
#undef PTD

#undef DETAIL_STINGRAY_DECLARE_CONSTRUCTOR_CREATOR

	template < typename InterfaceType, typename ClassType >
	struct DefaultConstructorCreator : public virtual ICreator<InterfaceType>
	{
		virtual shared_ptr<InterfaceType> Create() const { return make_shared<ClassType>(); }
	};

	template < typename InterfaceType, typename ClassType >
	shared_ptr<DefaultConstructorCreator<InterfaceType, ClassType> > MakeConstructorCreator()
	{ return make_shared<DefaultConstructorCreator<InterfaceType, ClassType> >(); }

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
