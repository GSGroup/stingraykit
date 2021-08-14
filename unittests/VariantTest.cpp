#include <stingraykit/log/Logger.h>
#include <stingraykit/VariantMultidispatch.h>
#include <stingraykit/variant.h>

#include <gtest/gtest.h>

using namespace stingray;

namespace
{

	struct Visitor : static_visitor<int>
	{
		template < typename T1, typename T2 >
		int operator () (const T1& p1, const T2& p2) const
		{
			Logger::Info() << "p1: " << Demangle(typeid(p1).name()) << " { " << p1 << " }, p2: " << Demangle(typeid(p2).name()) << " { " << p2 << " }";
			return Demangle(typeid(p1).name()).size();
		}
	};


	struct SVisitor : public static_visitor<int>
	{
		int operator () (const std::string&) const { return 0; }
		int operator () (int) const { return 1; }
	};

}


TEST(VariantTest, Variant)
{
	typedef variant<TypeList<int, std::string> > VType;
	VType v_default;
	VType v_int((int)42);
	VType v_string(std::string("1234"));

	ASSERT_TRUE(v_int.get_ptr<int>());
	ASSERT_TRUE(!v_int.get_ptr<std::string>());
	ASSERT_EQ(v_int.get<int>(), 42);
	ASSERT_EQ(v_int.which(), 0U);
	ASSERT_ANY_THROW(v_int.get<std::string>());

	ASSERT_TRUE(v_string.get_ptr<std::string>());
	ASSERT_TRUE(!v_string.get_ptr<int>());
	ASSERT_EQ(v_string.get<std::string>(), "1234");
	ASSERT_EQ(v_string.which(), 1U);
	ASSERT_ANY_THROW(v_string.get<int>());

	VType v_int_copy(v_int);
	ASSERT_TRUE(v_int_copy.get_ptr<int>());
	ASSERT_TRUE(!v_int_copy.get_ptr<std::string>());
	ASSERT_EQ(v_int_copy.get<int>(), 42);
	ASSERT_EQ(v_int_copy.which(), 0U);
	ASSERT_ANY_THROW(v_int_copy.get<std::string>());

	VType v_string_move(std::move(v_string));
	ASSERT_TRUE(v_string.get_ptr<std::string>());
	ASSERT_TRUE(v_string.get<std::string>().empty());
	ASSERT_TRUE(v_string_move.get_ptr<std::string>());
	ASSERT_EQ(v_string_move.get<std::string>(), "1234");

	VType v_emplace;
	ASSERT_TRUE(v_emplace.get_ptr<int>());
	v_emplace.emplace<std::string>("xxx", 1);
	ASSERT_TRUE(v_emplace.get_ptr<std::string>());
	ASSERT_EQ(v_emplace.get<std::string>(), "x");

	typedef variant<TypeList<EmptyType, int, std::string> > VEmpty;
	VEmpty v_empty;
	ASSERT_TRUE(v_empty.empty());
	v_empty.emplace<int>(42);
	ASSERT_TRUE(v_empty.get_ptr<int>());
	ASSERT_EQ(v_empty.get<int>(), 42);
}


TEST(VariantTest, VisitorApplier)
{
	typedef variant<TypeList<int, std::string> > VType;

	std::vector<VType> v;
	v.push_back(VType(std::string("1234")));
	v.push_back(VType(42));
	v.push_back(VType(0));

	std::vector<int> r;
	std::transform(v.begin(), v.end(), std::back_inserter(r), make_visitor_applier(SVisitor()));

	int seq[] = { 0, 1, 1 };
	ASSERT_TRUE(std::equal(r.begin(), r.end(), std::begin(seq), std::end(seq)));
}


TEST(VariantTest, Multidispatch)
{
//	typedef variant<TypeList<int, std::string> >		Variant1Type;
//	typedef variant<TypeList<char, double> >			Variant2Type;

//	Logger::Info() << "multivisitor result: " << Multidispatch(Visitor(), Variant1Type(1),						Variant2Type('z'));
//	Logger::Info() << "multivisitor result: " << Multidispatch(Visitor(), Variant1Type(std::string("test")),	Variant2Type(3.14));
//	Logger::Info() << "multivisitor result: " << Multidispatch(Visitor(), Variant1Type(1),						Variant2Type(3.14));
//	Logger::Info() << "multivisitor result: " << Multidispatch(Visitor(), Variant1Type(std::string("test")),	Variant2Type('z'));
}
