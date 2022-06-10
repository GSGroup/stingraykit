#include <stingraykit/VariantMultidispatch.h>
#include <stingraykit/variant.h>

#include <gmock/gmock-matchers.h>

using namespace stingray;

using testing::Pair;

namespace
{

	struct Visitor : static_visitor<std::pair<std::string, std::string>>
	{
		template < typename T1, typename T2 >
		std::pair<std::string, std::string> operator () (const T1& p1, const T2& p2) const
		{ return std::make_pair(ToString(p1), ToString(p2)); }
	};

	struct SVisitor : public static_visitor<int>
	{
		int operator () (const std::string&) const { return 0; }
		int operator () (int) const { return 1; }
	};

	template < size_t Index >
	class StringHolder
	{
	private:
		std::string	_str;

	public:
		StringHolder() : _str(ToString(Index)) { }

		const std::string& GetStr() const { return _str; }
	};

	struct StringHolderVisitor : public static_visitor<const std::string&>
	{
		template < typename StringHolder_ >
		const std::string& operator () (const StringHolder_& holder) const { return holder.GetStr(); }
	};

}


TEST(VariantTest, Variant)
{
	using VType = variant<TypeList<int, std::string>>;
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

	using VEmpty = variant<TypeList<EmptyType, int, std::string>>;
	VEmpty v_empty;
	ASSERT_TRUE(v_empty.empty());
	v_empty.emplace<int>(42);
	ASSERT_TRUE(v_empty.get_ptr<int>());
	ASSERT_EQ(v_empty.get<int>(), 42);
}


TEST(VariantTest, VisitorApplier)
{
	using VType = variant<TypeList<int, std::string>>;

	std::vector<VType> v;
	v.push_back(VType(std::string("1234")));
	v.push_back(VType(42));
	v.push_back(VType(0));

	std::vector<int> r;
	std::transform(v.begin(), v.end(), std::back_inserter(r), make_visitor_applier(SVisitor()));

	int seq[] = { 0, 1, 1 };
	ASSERT_TRUE(std::equal(r.begin(), r.end(), std::begin(seq), std::end(seq)));
}


TEST(VariantTest, VisitorApplierReturnRef)
{
	variant<TypeList<StringHolder<0>, StringHolder<3>, StringHolder<2>, StringHolder<5>>> var;

	ASSERT_EQ(&apply_visitor(StringHolderVisitor(), var), &var.get<StringHolder<0>>().GetStr());
	var = StringHolder<2>();
	ASSERT_EQ(&apply_visitor(StringHolderVisitor(), var), &var.get<StringHolder<2>>().GetStr());
	var = StringHolder<5>();
	ASSERT_EQ(&apply_visitor(StringHolderVisitor(), var), &var.get<StringHolder<5>>().GetStr());
	var = StringHolder<3>();
	ASSERT_EQ(&apply_visitor(StringHolderVisitor(), var), &var.get<StringHolder<3>>().GetStr());

}


TEST(VariantTest, Multidispatch)
{
	using Variant1Type = variant<TypeList<int, std::string>>;
	using Variant2Type = variant<TypeList<char, double>>;

	ASSERT_THAT(Multidispatch(Visitor(), Variant1Type(1), Variant2Type('z')), Pair("1", "z"));
	ASSERT_THAT(Multidispatch(Visitor(), Variant1Type(std::string("test")), Variant2Type(3.14)), Pair("test", "3.14"));
	ASSERT_THAT(Multidispatch(Visitor(), Variant1Type(1), Variant2Type(3.14)), Pair("1", "3.14"));
	ASSERT_THAT(Multidispatch(Visitor(), Variant1Type(std::string("test")), Variant2Type('z')), Pair("test", "z"));
}
