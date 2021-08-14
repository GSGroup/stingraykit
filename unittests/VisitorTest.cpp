#include <stingraykit/IVisitor.h>

#include <gtest/gtest.h>

using namespace stingray;

namespace
{

	struct IVisitableT : public virtual IVisitable<IVisitableT> { };
	struct VisitableT : public virtual IVisitableT, public Visitable<IVisitableT, VisitableT> { };
	struct UVisitableT : public virtual IVisitableT, public Visitable<IVisitableT, UVisitableT> { };

	struct IConstVisitableT : public virtual IVisitable<const IConstVisitableT> { };
	struct ConstVisitableT : public virtual IConstVisitableT, public Visitable<const IConstVisitableT, const ConstVisitableT> { };
	struct UConstVisitableT : public virtual IConstVisitableT, public Visitable<const IConstVisitableT, const UConstVisitableT> { };


	struct VisitableVisitorT : public Visitor<IVisitableT, VisitableT>
	{ void Visit(VisitableT& visitable) const { } };

	struct VisitableConstVisitorT : public Visitor<const IVisitableT, const VisitableT>
	{ void Visit(const VisitableT& visitable) const { } };

	struct ConstVisitableVisitorT : public Visitor<IConstVisitableT, ConstVisitableT>
	{ void Visit(ConstVisitableT& visitable) const { } };

	struct ConstVisitableConstVisitorT : public Visitor<const IConstVisitableT, const ConstVisitableT>
	{ void Visit(const ConstVisitableT& visitable) const { } };


	struct VisitableVisitorWithValueT : public Visitor<IVisitableT, VisitableT, int>
	{ int Visit(VisitableT& visitable) const { return 1; } };

	struct VisitableConstVisitorWithValueT : public Visitor<const IVisitableT, const VisitableT, int>
	{ int Visit(const VisitableT& visitable) const { return 2; } };

	struct ConstVisitableVisitorWithValueT : public Visitor<IConstVisitableT, ConstVisitableT, int>
	{ int Visit(ConstVisitableT& visitable) const { return 3; } };

	struct ConstVisitableConstVisitorWithValueT : public Visitor<const IConstVisitableT, const ConstVisitableT, int>
	{ int Visit(const ConstVisitableT& visitable) const { return 4; } };


	struct UVisitableVisitorWithValueT : public Visitor<IVisitableT, VisitableT, int>
	{
		int Visit(IVisitableT& visitable) const { return 0; }
		int Visit(VisitableT& visitable) const { return 1; }
	};

	struct UVisitableConstVisitorWithValueT : public Visitor<const IVisitableT, const VisitableT, int>
	{
		int Visit(const IVisitableT& visitable) const { return 0; }
		int Visit(const VisitableT& visitable) const { return 2; }
	};

	struct UConstVisitableVisitorWithValueT : public Visitor<IConstVisitableT, ConstVisitableT, int>
	{
		int Visit(IConstVisitableT& visitable) const { return 0; }
		int Visit(ConstVisitableT& visitable) const { return 3; }
	};

	struct UConstVisitableConstVisitorWithValueT : public Visitor<const IConstVisitableT, const ConstVisitableT, int>
	{
		int Visit(const IConstVisitableT& visitable) const { return 0; }
		int Visit(const ConstVisitableT& visitable) const { return 4; }
	};


	struct VisitableVisitorByPtrT : public Visitor<shared_ptr<IVisitableT>, shared_ptr<VisitableT> >
	{ void Visit(const shared_ptr<VisitableT>& visitable) const { } };

	struct VisitableConstVisitorByPtrT : public Visitor<shared_ptr<const IVisitableT>, shared_ptr<const VisitableT> >
	{ void Visit(const shared_ptr<const VisitableT>& visitable) const { } };

	struct ConstVisitableVisitorByPtrT : public Visitor<shared_ptr<IConstVisitableT>, shared_ptr<ConstVisitableT> >
	{ void Visit(const shared_ptr<ConstVisitableT>& visitable) const { } };

	struct ConstVisitableConstVisitorByPtrT : public Visitor<shared_ptr<const IConstVisitableT>, shared_ptr<const ConstVisitableT> >
	{ void Visit(const shared_ptr<const ConstVisitableT>& visitable) const { } };


	struct VisitableVisitorByPtrWithValueT : public Visitor<shared_ptr<IVisitableT>, shared_ptr<VisitableT>, int>
	{ int Visit(const shared_ptr<VisitableT>& visitable) const { return 1; } };

	struct VisitableConstVisitorByPtrWithValueT : public Visitor<shared_ptr<const IVisitableT>, shared_ptr<const VisitableT>, int>
	{ int Visit(const shared_ptr<const VisitableT>& visitable) const { return 2; } };

	struct ConstVisitableVisitorByPtrWithValueT : public Visitor<shared_ptr<IConstVisitableT>, shared_ptr<ConstVisitableT>, int>
	{ int Visit(const shared_ptr<ConstVisitableT>& visitable) const { return 3; } };

	struct ConstVisitableConstVisitorByPtrWithValueT : public Visitor<shared_ptr<const IConstVisitableT>, shared_ptr<const ConstVisitableT>, int>
	{ int Visit(const shared_ptr<const ConstVisitableT>& visitable) const { return 4; } };


	struct UVisitableVisitorByPtrWithValueT : public Visitor<shared_ptr<IVisitableT>, shared_ptr<VisitableT>, int>
	{
		int Visit(const shared_ptr<IVisitableT>& visitable) const { return 0; }
		int Visit(const shared_ptr<VisitableT>& visitable) const { return 1; }
	};

	struct UVisitableConstVisitorByPtrWithValueT : public Visitor<shared_ptr<const IVisitableT>, shared_ptr<const VisitableT>, int>
	{
		int Visit(const shared_ptr<const IVisitableT>& visitable) const { return 0; }
		int Visit(const shared_ptr<const VisitableT>& visitable) const { return 2; }
	};

	struct UConstVisitableVisitorByPtrWithValueT : public Visitor<shared_ptr<IConstVisitableT>, shared_ptr<ConstVisitableT>, int>
	{
		int Visit(const shared_ptr<IConstVisitableT>& visitable) const { return 0; }
		int Visit(const shared_ptr<ConstVisitableT>& visitable) const { return 3; }
	};

	struct UConstVisitableConstVisitorByPtrWithValueT : public Visitor<shared_ptr<const IConstVisitableT>, shared_ptr<const ConstVisitableT>, int>
	{
		int Visit(const shared_ptr<const IConstVisitableT>& visitable) const { return 0; }
		int Visit(const shared_ptr<const ConstVisitableT>& visitable) const { return 4; }
	};

}


TEST(VisitorTest, Visitor)
{
	VisitableT v;
	UVisitableT uv;

	const ConstVisitableT& cv = ConstVisitableT();
	const UConstVisitableT& ucv = UConstVisitableT();

	VisitableVisitorT vv;

	VisitableConstVisitorT vcv;

//	ConstVisitableVisitorT cvv;
	ConstVisitableConstVisitorT cvcv;

	ApplyVisitor(vv, v);
	ApplyVisitor(vcv, v);

//	ApplyVisitor(cvv, cv);
	ApplyVisitor(cvcv, cv);

	ASSERT_ANY_THROW(ApplyVisitor(vv, uv));
	ASSERT_ANY_THROW(ApplyVisitor(vcv, uv));

//	ASSERT_ANY_THROW(ApplyVisitor(cvv, ucv));
	ASSERT_ANY_THROW(ApplyVisitor(cvcv, ucv));
}


TEST(VisitorTest, VisitorWithValue)
{
	VisitableT v;
	UVisitableT uv;

	const ConstVisitableT& cv = ConstVisitableT();
	const UConstVisitableT& ucv = UConstVisitableT();

	VisitableVisitorWithValueT vv;
	VisitableConstVisitorWithValueT vcv;

//	ConstVisitableVisitorWithValueT cvv;
	ConstVisitableConstVisitorWithValueT cvcv;

	UVisitableVisitorWithValueT uvv;
	UVisitableConstVisitorWithValueT uvcv;

//	UConstVisitableVisitorWithValueT ucvv;
	UConstVisitableConstVisitorWithValueT ucvcv;

	ASSERT_EQ(ApplyVisitor(vv, v), 1);
	ASSERT_EQ(ApplyVisitor(vcv, v), 2);

//	ASSERT_EQ(ApplyVisitor(cvv, cv), 3);
	ASSERT_EQ(ApplyVisitor(cvcv, cv), 4);

	ASSERT_ANY_THROW(ApplyVisitor(vv, uv));
	ASSERT_ANY_THROW(ApplyVisitor(vcv, uv));

//	ASSERT_ANY_THROW(ApplyVisitor(cvv, ucv));
	ASSERT_ANY_THROW(ApplyVisitor(cvcv, ucv));

	ASSERT_EQ(ApplyVisitor(uvv, uv), 0);
	ASSERT_EQ(ApplyVisitor(uvcv, uv), 0);

//	ASSERT_EQ(ApplyVisitor(ucvv, ucv), 0);
	ASSERT_EQ(ApplyVisitor(ucvcv, ucv), 0);
}


TEST(VisitorTest, VisitorByPtr)
{
	shared_ptr<IVisitableT> v(new VisitableT);
	shared_ptr<IVisitableT> uv(new UVisitableT);
	shared_ptr<IVisitableT> nv;

	shared_ptr<const IConstVisitableT> cv(new ConstVisitableT);
	shared_ptr<const IConstVisitableT> ucv(new UConstVisitableT);
	shared_ptr<const IConstVisitableT> ncv;

	VisitableVisitorByPtrT vv;
	VisitableConstVisitorByPtrT vcv;

//	ConstVisitableVisitorByPtrT cvv;
	ConstVisitableConstVisitorByPtrT cvcv;

	ApplyVisitor(vv, v);
	ApplyVisitor(vcv, v);

//	ApplyVisitor(cvv, cv);
	ApplyVisitor(cvcv, cv);

	ASSERT_ANY_THROW(ApplyVisitor(vv, uv));
	ASSERT_ANY_THROW(ApplyVisitor(vcv, uv));

//	ASSERT_ANY_THROW(ApplyVisitor(cvv, ucv));
	ASSERT_ANY_THROW(ApplyVisitor(cvcv, ucv));

	ASSERT_ANY_THROW(ApplyVisitor(vv, nv));
	ASSERT_ANY_THROW(ApplyVisitor(vcv, nv));

//	ASSERT_ANY_THROW(ApplyVisitor(cvv, ncv));
	ASSERT_ANY_THROW(ApplyVisitor(cvcv, ncv));
}


TEST(VisitorTest, VisitorByPtrWithValue)
{
	shared_ptr<IVisitableT> v(new VisitableT);
	shared_ptr<IVisitableT> uv(new UVisitableT);
	shared_ptr<IVisitableT> nv;

	shared_ptr<const IConstVisitableT> cv(new ConstVisitableT);
	shared_ptr<const IConstVisitableT> ucv(new UConstVisitableT);
	shared_ptr<const IConstVisitableT> ncv;

	VisitableVisitorByPtrWithValueT vv;
	VisitableConstVisitorByPtrWithValueT vcv;

//	ConstVisitableVisitorByPtrWithValueT cvv;
	ConstVisitableConstVisitorByPtrWithValueT cvcv;

	UVisitableVisitorByPtrWithValueT uvv;
	UVisitableConstVisitorByPtrWithValueT uvcv;

//	UConstVisitableVisitorByPtrWithValueT ucvv;
	UConstVisitableConstVisitorByPtrWithValueT ucvcv;

	ASSERT_EQ(ApplyVisitor(vv, v), 1);
	ASSERT_EQ(ApplyVisitor(vcv, v), 2);

//	ASSERT_EQ(ApplyVisitor(cvv, cv), 3);
	ASSERT_EQ(ApplyVisitor(cvcv, cv), 4);

	ASSERT_ANY_THROW(ApplyVisitor(vv, uv));
	ASSERT_ANY_THROW(ApplyVisitor(vcv, uv));

//	ASSERT_ANY_THROW(ApplyVisitor(cvv, ucv));
	ASSERT_ANY_THROW(ApplyVisitor(cvcv, ucv));

	ASSERT_ANY_THROW(ApplyVisitor(vv, nv));
	ASSERT_ANY_THROW(ApplyVisitor(vcv, nv));

//	ASSERT_ANY_THROW(ApplyVisitor(cvv, ncv));
	ASSERT_ANY_THROW(ApplyVisitor(cvcv, ncv));

	ASSERT_EQ(ApplyVisitor(uvv, uv), 0);
	ASSERT_EQ(ApplyVisitor(uvcv, uv), 0);

//	ASSERT_EQ(ApplyVisitor(ucvv, ucv), 0);
	ASSERT_EQ(ApplyVisitor(ucvcv, ucv), 0);
}


TEST(VisitorTest, VisitorByPtrApplier)
{
	std::vector<shared_ptr<IVisitableT> > v;
	v.push_back(make_shared_ptr<VisitableT>());
	v.push_back(make_shared_ptr<UVisitableT>());
	v.push_back(make_shared_ptr<VisitableT>());

	UVisitableVisitorByPtrWithValueT vv;

	std::vector<int> r;
	std::transform(v.begin(), v.end(), std::back_inserter(r), MakeVisitorApplier(vv));

	int seq[] = { 1, 0, 1 };
	ASSERT_TRUE(std::equal(r.begin(), r.end(), std::begin(seq), std::end(seq)));
}
