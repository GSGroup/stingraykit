// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/optional.h>

#include <gtest/gtest.h>

using namespace stingray;

namespace
{

	struct OptionalHelper
	{
		u32		Value;
		bool&	Destroyed;

		OptionalHelper(u32 value, bool &dtorFlag) :
			Value(value),
			Destroyed(dtorFlag)
		{ }

		~OptionalHelper()
		{ Destroyed = true; }
	};

	struct ObjectExplicit
	{
		STINGRAYKIT_DEFAULTCOPYABLE(ObjectExplicit);
		STINGRAYKIT_DEFAULTMOVABLE(ObjectExplicit);

		size_t			Val = 0;
		std::string		Data;

		explicit ObjectExplicit(const std::string& data)
			:	Data(data)
		{ }

		explicit ObjectExplicit(std::initializer_list<int> il, const std::string& data)
			:	Val(il.size()),
				Data(data)
		{ }

		explicit ObjectExplicit(std::string&& data)
			:	Data(std::move(data))
		{ }

		explicit ObjectExplicit(std::initializer_list<int> il, std::string&& data)
			:	Val(il.size()),
				Data(std::move(data))
		{ }

		ObjectExplicit& operator = (const std::string& data)
		{ Data = data; return *this; }

		ObjectExplicit& operator = (std::string&& data)
		{ Data = std::move(data); return *this; }

		explicit operator std::string () const &
		{ return Data; }

		explicit operator std::string () &&
		{ return std::move(Data); }
	};

	struct ObjectImplicit
	{
		STINGRAYKIT_DEFAULTCOPYABLE(ObjectImplicit);
		STINGRAYKIT_DEFAULTMOVABLE(ObjectImplicit);

		std::string		Data;

		ObjectImplicit(const std::string& data)
			:	Data(data)
		{ }

		ObjectImplicit(std::string&& data)
			:	Data(std::move(data))
		{ }

		ObjectImplicit& operator = (const std::string& data)
		{ Data = data; return *this; }

		ObjectImplicit& operator = (std::string&& data)
		{ Data = std::move(data); return *this; }

		operator std::string () const &
		{ return Data; }

		operator std::string () &&
		{ return std::move(Data); }
	};

	struct Unswappable
	{ };

	void swap(Unswappable&, Unswappable&)
	{ STINGRAYKIT_THROW(InvalidOperationException()); }

	template < typename T >
	struct InstantFailToConvert
	{
		InstantFailToConvert()
		{ }

		operator T () const
		{ STINGRAYKIT_THROW(InvalidOperationException()); }
	};

}


TEST(OptionalTest, Test)
{
	optional<OptionalHelper> opt;
	ASSERT_TRUE(!opt);
	bool dtor_flag = false;
	opt.emplace(42, wrap_ref(dtor_flag));
	ASSERT_TRUE(opt);
	ASSERT_TRUE(!dtor_flag);
	ASSERT_EQ(opt->Value, 42u);
	opt = null;
	ASSERT_TRUE(!opt);
	ASSERT_TRUE(dtor_flag);
	ASSERT_ANY_THROW(*opt);
}


TEST(OptionalTest, ExplicitCtor)
{
	{
		ObjectExplicit obj("o1");

		{
			const optional<ObjectExplicit> opt(obj);
			ASSERT_TRUE(opt);
			ASSERT_EQ(opt->Data, "o1");
		}

		ASSERT_EQ(obj.Data, "o1");

		{
			const optional<ObjectExplicit> opt(std::move(obj));
			ASSERT_TRUE(opt);
			ASSERT_EQ(opt->Data, "o1");
		}

		ASSERT_TRUE(obj.Data.empty());
	}

	{
		std::string str("s1");

		{
			const optional<ObjectExplicit> opt(str);
			ASSERT_TRUE(opt);
			ASSERT_EQ(opt->Data, "s1");
		}

		ASSERT_TRUE(str == "s1");

		{
			const optional<ObjectExplicit> opt(std::move(str));
			ASSERT_TRUE(opt);
			ASSERT_EQ(opt->Data, "s1");
		}

		ASSERT_TRUE(str.empty());
	}

	{
		optional<ObjectExplicit> opt("o1");

		{
			const optional<ObjectExplicit> opt2(opt);
			ASSERT_TRUE(opt2);
			ASSERT_EQ(opt2->Data, "o1");
		}

		ASSERT_TRUE(opt);
		ASSERT_EQ(opt->Data, "o1");

		{
			const optional<ObjectExplicit> opt2(std::move(opt));
			ASSERT_TRUE(opt2);
			ASSERT_EQ(opt2->Data, "o1");
		}

		ASSERT_TRUE(opt);
		ASSERT_TRUE(opt->Data.empty());
	}

	{
		optional<std::string> opt("o1");

		{
			const optional<ObjectExplicit> opt2(opt);
			ASSERT_TRUE(opt2);
			ASSERT_EQ(opt2->Data, "o1");
		}

		ASSERT_TRUE(opt);
		ASSERT_EQ(*opt, "o1");

		{
			const optional<ObjectExplicit> opt2(std::move(opt));
			ASSERT_TRUE(opt2);
			ASSERT_EQ(opt2->Data, "o1");
		}

		ASSERT_TRUE(opt);
		ASSERT_TRUE(opt->empty());
	}

	{
		optional<ObjectExplicit> opt("o1");

		{
			const optional<std::string> opt2(opt);
			ASSERT_TRUE(opt2);
			ASSERT_EQ(*opt2, "o1");
		}

		ASSERT_TRUE(opt);
		ASSERT_EQ(opt->Data, "o1");

		{
			const optional<std::string> opt2(std::move(opt));
			ASSERT_TRUE(opt2);
			ASSERT_EQ(*opt2, "o1");
		}

		ASSERT_TRUE(opt);
		ASSERT_TRUE(opt->Data.empty());
	}
}


TEST(OptionalTest, ExplicitCtorInPlace)
{
	{
		ObjectExplicit obj("o1");

		{
			const optional<ObjectExplicit> opt(InPlace, obj);
			ASSERT_TRUE(opt);
			ASSERT_EQ(opt->Data, "o1");
		}

		ASSERT_EQ(obj.Data, "o1");

		{
			const optional<ObjectExplicit> opt(InPlace, std::move(obj));
			ASSERT_TRUE(opt);
			ASSERT_EQ(opt->Data, "o1");
		}

		ASSERT_TRUE(obj.Data.empty());
	}

	{
		std::string str("s1");

		{
			const optional<ObjectExplicit> opt(InPlace, str);
			ASSERT_TRUE(opt);
			ASSERT_EQ(opt->Data, "s1");
		}

		ASSERT_TRUE(str == "s1");

		{
			const optional<ObjectExplicit> opt(InPlace, std::move(str));
			ASSERT_TRUE(opt);
			ASSERT_EQ(opt->Data, "s1");
		}

		ASSERT_TRUE(str.empty());
	}

	{
		std::string str("s1");

		{
			const optional<ObjectExplicit> opt(InPlace, { 1 }, str);
			ASSERT_TRUE(opt);
			ASSERT_EQ(opt->Val, 1u);
			ASSERT_EQ(opt->Data, "s1");
		}

		ASSERT_TRUE(str == "s1");

		{
			const optional<ObjectExplicit> opt(InPlace, { 1, 2 }, std::move(str));
			ASSERT_TRUE(opt);
			ASSERT_EQ(opt->Val, 2u);
			ASSERT_EQ(opt->Data, "s1");
		}

		ASSERT_TRUE(str.empty());
	}
}


TEST(OptionalTest, ExplicitAssign)
{
	{
		ObjectExplicit obj("o1");

		{
			optional<ObjectExplicit> opt;
			opt = obj;
			ASSERT_TRUE(opt);
			ASSERT_EQ(opt->Data, "o1");
		}

		ASSERT_EQ(obj.Data, "o1");

		{
			optional<ObjectExplicit> opt;
			opt = std::move(obj);
			ASSERT_TRUE(opt);
			ASSERT_EQ(opt->Data, "o1");
		}

		ASSERT_TRUE(obj.Data.empty());
	}

	{
		std::string str("s1");

		{
			optional<ObjectExplicit> opt;
			opt = str;
			ASSERT_TRUE(opt);
			ASSERT_EQ(opt->Data, "s1");
		}

		ASSERT_TRUE(str == "s1");

		{
			optional<ObjectExplicit> opt;
			opt = std::move(str);
			ASSERT_TRUE(opt);
			ASSERT_EQ(opt->Data, "s1");
		}

		ASSERT_TRUE(str.empty());
	}

	{
		optional<ObjectExplicit> opt("o1");

		{
			optional<ObjectExplicit> opt2;
			opt2 = opt;
			ASSERT_TRUE(opt2);
			ASSERT_EQ(opt2->Data, "o1");
		}

		ASSERT_TRUE(opt);
		ASSERT_EQ(opt->Data, "o1");

		{
			optional<ObjectExplicit> opt2;
			opt2 = std::move(opt);
			ASSERT_TRUE(opt2);
			ASSERT_EQ(opt2->Data, "o1");
		}

		ASSERT_TRUE(opt);
		ASSERT_TRUE(opt->Data.empty());
	}

	{
		optional<std::string> opt("o1");

		{
			optional<ObjectExplicit> opt2;
			opt2 = opt;
			ASSERT_TRUE(opt2);
			ASSERT_EQ(opt2->Data, "o1");
		}

		ASSERT_TRUE(opt);
		ASSERT_EQ(*opt, "o1");

		{
			optional<ObjectExplicit> opt2;
			opt2 = std::move(opt);
			ASSERT_TRUE(opt2);
			ASSERT_EQ(opt2->Data, "o1");
		}

		ASSERT_TRUE(opt);
		ASSERT_TRUE(opt->empty());
	}
}


TEST(OptionalTest, ExplicitEmplace)
{
	{
		ObjectExplicit obj("o1");

		{
			optional<ObjectExplicit> opt;
			opt.emplace(obj);
			ASSERT_TRUE(opt);
			ASSERT_EQ(opt->Data, "o1");
		}

		ASSERT_EQ(obj.Data, "o1");

		{
			optional<ObjectExplicit> opt;
			opt.emplace(std::move(obj));
			ASSERT_TRUE(opt);
			ASSERT_EQ(opt->Data, "o1");
		}

		ASSERT_TRUE(obj.Data.empty());
	}

	{
		std::string str("s1");

		{
			optional<ObjectExplicit> opt;
			opt.emplace(str);
			ASSERT_TRUE(opt);
			ASSERT_EQ(opt->Data, "s1");
		}

		ASSERT_TRUE(str == "s1");

		{
			optional<ObjectExplicit> opt;
			opt.emplace(std::move(str));
			ASSERT_TRUE(opt);
			ASSERT_EQ(opt->Data, "s1");
		}

		ASSERT_TRUE(str.empty());
	}

	{
		std::string str("s1");

		{
			optional<ObjectExplicit> opt;
			opt.emplace({ 1 }, str);
			ASSERT_TRUE(opt);
			ASSERT_EQ(opt->Val, 1u);
			ASSERT_EQ(opt->Data, "s1");
		}

		ASSERT_TRUE(str == "s1");

		{
			optional<ObjectExplicit> opt;
			opt.emplace({ 1, 2 }, std::move(str));
			ASSERT_TRUE(opt);
			ASSERT_EQ(opt->Val, 2u);
			ASSERT_EQ(opt->Data, "s1");
		}

		ASSERT_TRUE(str.empty());
	}
}


TEST(OptionalTest, ExplicitMakeOptional)
{
	{
		ObjectExplicit obj("o1");

		{
			const auto opt = make_optional_value(obj);
			ASSERT_TRUE(opt);
			ASSERT_EQ(opt->Data, "o1");
		}

		ASSERT_EQ(obj.Data, "o1");

		{
			const auto opt = make_optional_value(std::move(obj));
			ASSERT_TRUE(opt);
			ASSERT_EQ(opt->Data, "o1");
		}

		ASSERT_TRUE(obj.Data.empty());
	}

	{
		std::string str("s1");

		{
			const auto opt = make_optional_value<ObjectExplicit>(str);
			ASSERT_TRUE(opt);
			ASSERT_EQ(opt->Data, "s1");
		}

		ASSERT_TRUE(str == "s1");

		{
			const auto opt = make_optional_value<ObjectExplicit>(std::move(str));
			ASSERT_TRUE(opt);
			ASSERT_EQ(opt->Data, "s1");
		}

		ASSERT_TRUE(str.empty());
	}

	{
		std::string str("s1");

		{
			const auto opt = make_optional_value<ObjectExplicit>({ 1 }, str);
			ASSERT_TRUE(opt);
			ASSERT_EQ(opt->Val, 1u);
			ASSERT_EQ(opt->Data, "s1");
		}

		ASSERT_TRUE(str == "s1");

		{
			const auto opt = make_optional_value<ObjectExplicit>({ 1, 2 }, std::move(str));
			ASSERT_TRUE(opt);
			ASSERT_EQ(opt->Val, 2u);
			ASSERT_EQ(opt->Data, "s1");
		}

		ASSERT_TRUE(str.empty());
	}
}


TEST(OptionalTest, ImplicitCtor)
{
	{
		ObjectImplicit obj("o1");

		{
			const optional<ObjectImplicit> opt = obj;
			ASSERT_TRUE(opt);
			ASSERT_EQ(opt->Data, "o1");
		}

		ASSERT_EQ(obj.Data, "o1");

		{
			const optional<ObjectImplicit> opt = std::move(obj);
			ASSERT_TRUE(opt);
			ASSERT_EQ(opt->Data, "o1");
		}

		ASSERT_TRUE(obj.Data.empty());
	}

	{
		std::string str("s1");

		{
			const optional<ObjectImplicit> opt = str;
			ASSERT_TRUE(opt);
			ASSERT_EQ(opt->Data, "s1");
		}

		ASSERT_TRUE(str == "s1");

		{
			const optional<ObjectImplicit> opt = std::move(str);
			ASSERT_TRUE(opt);
			ASSERT_EQ(opt->Data, "s1");
		}

		ASSERT_TRUE(str.empty());
	}

	{
		optional<ObjectImplicit> opt("o1");

		{
			const optional<ObjectImplicit> opt2 = opt;
			ASSERT_TRUE(opt2);
			ASSERT_EQ(opt2->Data, "o1");
		}

		ASSERT_TRUE(opt);
		ASSERT_EQ(opt->Data, "o1");

		{
			const optional<ObjectImplicit> opt2 = std::move(opt);
			ASSERT_TRUE(opt2);
			ASSERT_EQ(opt2->Data, "o1");
		}

		ASSERT_TRUE(opt);
		ASSERT_TRUE(opt->Data.empty());
	}

	{
		optional<std::string> opt("o1");

		{
			const optional<ObjectImplicit> opt2 = opt;
			ASSERT_TRUE(opt2);
			ASSERT_EQ(opt2->Data, "o1");
		}

		ASSERT_TRUE(opt);
		ASSERT_EQ(*opt, "o1");

		{
			const optional<ObjectImplicit> opt2 = std::move(opt);
			ASSERT_TRUE(opt2);
			ASSERT_EQ(opt2->Data, "o1");
		}

		ASSERT_TRUE(opt);
		ASSERT_TRUE(opt->empty());
	}

	{
		optional<ObjectImplicit> opt("o1");

		{
			const optional<std::string> opt2 = opt;
			ASSERT_TRUE(opt2);
			ASSERT_EQ(*opt2, "o1");
		}

		ASSERT_TRUE(opt);
		ASSERT_EQ(opt->Data, "o1");

		{
			const optional<std::string> opt2 = std::move(opt);
			ASSERT_TRUE(opt2);
			ASSERT_EQ(*opt2, "o1");
		}

		ASSERT_TRUE(opt);
		ASSERT_TRUE(opt->Data.empty());
	}
}


TEST(OptionalTest, ImplicitAssign)
{
	{
		ObjectImplicit obj("o1");

		{
			optional<ObjectImplicit> opt;
			opt = obj;
			ASSERT_TRUE(opt);
			ASSERT_EQ(opt->Data, "o1");
		}

		ASSERT_EQ(obj.Data, "o1");

		{
			optional<ObjectImplicit> opt;
			opt = std::move(obj);
			ASSERT_TRUE(opt);
			ASSERT_EQ(opt->Data, "o1");
		}

		ASSERT_TRUE(obj.Data.empty());
	}

	{
		std::string str("s1");

		{
			optional<ObjectImplicit> opt;
			opt = str;
			ASSERT_TRUE(opt);
			ASSERT_EQ(opt->Data, "s1");
		}

		ASSERT_TRUE(str == "s1");

		{
			optional<ObjectImplicit> opt;
			opt = std::move(str);
			ASSERT_TRUE(opt);
			ASSERT_EQ(opt->Data, "s1");
		}

		ASSERT_TRUE(str.empty());
	}

	{
		optional<ObjectImplicit> opt("o1");

		{
			optional<ObjectImplicit> opt2;
			opt2 = opt;
			ASSERT_TRUE(opt2);
			ASSERT_EQ(opt2->Data, "o1");
		}

		ASSERT_TRUE(opt);
		ASSERT_EQ(opt->Data, "o1");

		{
			optional<ObjectImplicit> opt2;
			opt2 = std::move(opt);
			ASSERT_TRUE(opt2);
			ASSERT_EQ(opt2->Data, "o1");
		}

		ASSERT_TRUE(opt);
		ASSERT_TRUE(opt->Data.empty());
	}

	{
		optional<std::string> opt("o1");

		{
			optional<ObjectImplicit> opt2;
			opt2 = opt;
			ASSERT_TRUE(opt2);
			ASSERT_EQ(opt2->Data, "o1");
		}

		ASSERT_TRUE(opt);
		ASSERT_EQ(*opt, "o1");

		{
			optional<ObjectImplicit> opt2;
			opt2 = std::move(opt);
			ASSERT_TRUE(opt2);
			ASSERT_EQ(opt2->Data, "o1");
		}

		ASSERT_TRUE(opt);
		ASSERT_TRUE(opt->empty());
	}
}


TEST(OptionalTest, Dereference)
{
	{
		optional<std::string> opt("o1");

		{
			const std::string s1 = opt.get();
			ASSERT_EQ(s1, "o1");
		}

		ASSERT_TRUE(opt);
		ASSERT_EQ(*opt, "o1");

		{
			const std::string s1 = std::move(opt).get();
			ASSERT_EQ(s1, "o1");
		}

		ASSERT_TRUE(opt);
		ASSERT_TRUE(opt->empty());
	}

	{
		optional<std::string> opt("o1");

		{
			const std::string s1 = *opt;
			ASSERT_EQ(s1, "o1");
		}

		ASSERT_TRUE(opt);
		ASSERT_EQ(*opt, "o1");

		{
			const std::string s1 = *std::move(opt);
			ASSERT_EQ(s1, "o1");
		}

		ASSERT_TRUE(opt);
		ASSERT_TRUE(opt->empty());
	}
}


TEST(OptionalTest, ConditionalDereference)
{
	{
		optional<std::string> opt("o1");

		{
			const std::string s1 = opt.get_value_or("o2");
			ASSERT_EQ(s1, "o1");
		}

		ASSERT_TRUE(opt);
		ASSERT_EQ(*opt, "o1");

		{
			const std::string s1 = std::move(opt).get_value_or("o2");
			ASSERT_EQ(s1, "o1");
		}

		ASSERT_TRUE(opt);
		ASSERT_TRUE(opt->empty());
	}

	{
		optional<std::string> opt;

		{
			const std::string s1 = opt.get_value_or("o2");
			ASSERT_EQ(s1, "o2");
		}

		ASSERT_FALSE(opt);

		{
			const std::string s1 = std::move(opt).get_value_or("o2");
			ASSERT_EQ(s1, "o2");
		}

		ASSERT_FALSE(opt);
	}

	{
		optional<std::string> opt("o1");

		{
			std::string s1;
			ASSERT_NO_THROW(s1 = opt.get_value_or(InstantFailToConvert<std::string>()));
			ASSERT_EQ(s1, "o1");
		}

		ASSERT_TRUE(opt);
		ASSERT_EQ(*opt, "o1");

		{
			std::string s1;
			ASSERT_NO_THROW(s1 = std::move(opt).get_value_or(InstantFailToConvert<std::string>()));
			ASSERT_EQ(s1, "o1");
		}

		ASSERT_TRUE(opt);
		ASSERT_TRUE(opt->empty());
	}

	{
		optional<std::string> opt;

		{
			std::string s1;
			ASSERT_ANY_THROW(s1 = opt.get_value_or(InstantFailToConvert<std::string>()));
			ASSERT_TRUE(s1.empty());
		}

		ASSERT_FALSE(opt);

		{
			std::string s1;
			ASSERT_ANY_THROW(s1 = std::move(opt).get_value_or(InstantFailToConvert<std::string>()));
			ASSERT_TRUE(s1.empty());
		}

		ASSERT_FALSE(opt);
	}
}


TEST(OptionalTest, Swap)
{
	{
		optional<Unswappable> opt1(InPlace);
		optional<Unswappable> opt2(InPlace);

		ASSERT_TRUE(opt1);
		ASSERT_TRUE(opt2);

		ASSERT_ANY_THROW(opt1.swap(opt2));

		ASSERT_TRUE(opt1);
		ASSERT_TRUE(opt2);
	}

	{
		optional<std::string> opt1("o1");
		optional<std::string> opt2("o2");

		ASSERT_TRUE(opt1);
		ASSERT_EQ(*opt1, "o1");
		ASSERT_TRUE(opt2);
		ASSERT_EQ(*opt2, "o2");

		opt1.swap(opt2);

		ASSERT_TRUE(opt1);
		ASSERT_EQ(*opt1, "o2");
		ASSERT_TRUE(opt2);
		ASSERT_EQ(*opt2, "o1");
	}

	{
		optional<std::string> opt1("o1");
		optional<std::string> opt2;

		ASSERT_TRUE(opt1);
		ASSERT_EQ(*opt1, "o1");
		ASSERT_FALSE(opt2);

		opt1.swap(opt2);

		ASSERT_FALSE(opt1);
		ASSERT_TRUE(opt2);
		ASSERT_EQ(*opt2, "o1");
	}

	{
		optional<std::string> opt1;
		optional<std::string> opt2("o2");

		ASSERT_FALSE(opt1);
		ASSERT_TRUE(opt2);
		ASSERT_EQ(*opt2, "o2");

		opt1.swap(opt2);

		ASSERT_TRUE(opt1);
		ASSERT_EQ(*opt1, "o2");
		ASSERT_FALSE(opt2);
	}

	{
		optional<std::string> opt1;
		optional<std::string> opt2;

		ASSERT_FALSE(opt1);
		ASSERT_FALSE(opt2);

		opt1.swap(opt2);

		ASSERT_FALSE(opt1);
		ASSERT_FALSE(opt2);
	}
}


#define CHECK_OPTIONAL_EQUALITY_TRUE(EqualityOp, Lhs, Rhs) \
		do { \
			ASSERT_TRUE(Lhs EqualityOp Rhs); \
			ASSERT_TRUE(Rhs EqualityOp Lhs); \
		} while (false)

#define CHECK_OPTIONAL_EQUALITY_FALSE(EqualityOp, Lhs, Rhs) \
		do { \
			ASSERT_FALSE(Lhs EqualityOp Rhs); \
			ASSERT_FALSE(Rhs EqualityOp Lhs); \
		} while (false)

TEST(OptionalTest, EqualityOps)
{
	const int v1 = 1;
	const long long v2 = 2;
	const optional<int> optn;
	const optional<int> optn_;
	const optional<int> opt1(1);
	const optional<int> opt1_(1);
	const optional<long long> opt2(2);

	{
		CHECK_OPTIONAL_EQUALITY_TRUE(==, optn, null);
		CHECK_OPTIONAL_EQUALITY_FALSE(==, optn, v1);
		CHECK_OPTIONAL_EQUALITY_FALSE(==, optn, v2);
		CHECK_OPTIONAL_EQUALITY_TRUE(==, optn, optn_);
		CHECK_OPTIONAL_EQUALITY_FALSE(==, optn, opt1);
		CHECK_OPTIONAL_EQUALITY_FALSE(==, optn, opt2);

		CHECK_OPTIONAL_EQUALITY_FALSE(==, opt1, null);
		CHECK_OPTIONAL_EQUALITY_TRUE(==, opt1, v1);
		CHECK_OPTIONAL_EQUALITY_FALSE(==, opt1, v2);
		CHECK_OPTIONAL_EQUALITY_FALSE(==, opt1, optn);
		CHECK_OPTIONAL_EQUALITY_TRUE(==, opt1, opt1_);
		CHECK_OPTIONAL_EQUALITY_FALSE(==, opt1, opt2);

		CHECK_OPTIONAL_EQUALITY_FALSE(==, opt2, null);
		CHECK_OPTIONAL_EQUALITY_FALSE(==, opt2, v1);
		CHECK_OPTIONAL_EQUALITY_TRUE(==, opt2, v2);
		CHECK_OPTIONAL_EQUALITY_FALSE(==, opt2, optn);
		CHECK_OPTIONAL_EQUALITY_FALSE(==, opt2, opt1);
	}

	{
		CHECK_OPTIONAL_EQUALITY_FALSE(!=, optn, null);
		CHECK_OPTIONAL_EQUALITY_TRUE(!=, optn, v1);
		CHECK_OPTIONAL_EQUALITY_TRUE(!=, optn, v2);
		CHECK_OPTIONAL_EQUALITY_FALSE(!=, optn, optn_);
		CHECK_OPTIONAL_EQUALITY_TRUE(!=, optn, opt1);
		CHECK_OPTIONAL_EQUALITY_TRUE(!=, optn, opt2);

		CHECK_OPTIONAL_EQUALITY_TRUE(!=, opt1, null);
		CHECK_OPTIONAL_EQUALITY_FALSE(!=, opt1, v1);
		CHECK_OPTIONAL_EQUALITY_TRUE(!=, opt1, v2);
		CHECK_OPTIONAL_EQUALITY_TRUE(!=, opt1, optn);
		CHECK_OPTIONAL_EQUALITY_FALSE(!=, opt1, opt1_);
		CHECK_OPTIONAL_EQUALITY_TRUE(!=, opt1, opt2);

		CHECK_OPTIONAL_EQUALITY_TRUE(!=, opt2, null);
		CHECK_OPTIONAL_EQUALITY_TRUE(!=, opt2, v1);
		CHECK_OPTIONAL_EQUALITY_FALSE(!=, opt2, v2);
		CHECK_OPTIONAL_EQUALITY_TRUE(!=, opt2, optn);
		CHECK_OPTIONAL_EQUALITY_TRUE(!=, opt2, opt1);
	}
}


#define CHECK_OPTIONAL_RELATIONAL_TRUE_TRUE(RelationalOp, OppositeOp, Lhs, Rhs) \
		do { \
			ASSERT_TRUE(Lhs RelationalOp Rhs); \
			ASSERT_TRUE(Rhs OppositeOp Lhs); \
		} while (false)

#define CHECK_OPTIONAL_RELATIONAL_FALSE_FALSE(RelationalOp, OppositeOp, Lhs, Rhs) \
		do { \
			ASSERT_FALSE(Lhs RelationalOp Rhs); \
			ASSERT_FALSE(Rhs OppositeOp Lhs); \
		} while (false)

#define CHECK_OPTIONAL_RELATIONAL_TRUE_FALSE(RelationalOp, OppositeOp, Lhs, Rhs) \
		do { \
			ASSERT_TRUE(Lhs RelationalOp Rhs); \
			ASSERT_FALSE(Rhs OppositeOp Lhs); \
		} while (false)

#define CHECK_OPTIONAL_RELATIONAL_FALSE_TRUE(RelationalOp, OppositeOp, Lhs, Rhs) \
		do { \
			ASSERT_FALSE(Lhs RelationalOp Rhs); \
			ASSERT_TRUE(Rhs OppositeOp Lhs); \
		} while (false)

TEST(OptionalTest, RelationalOps)
{
	const int v1 = 1;
	const long long v2 = 2;
	const optional<int> optn;
	const optional<int> optn_;
	const optional<int> opt1(1);
	const optional<int> opt1_(1);
	const optional<long long> opt2(2);

	{
		CHECK_OPTIONAL_RELATIONAL_FALSE_TRUE(<, >=, optn, null);
		CHECK_OPTIONAL_RELATIONAL_TRUE_TRUE(<, >=, optn, v1);
		CHECK_OPTIONAL_RELATIONAL_TRUE_TRUE(<, >=, optn, v2);
		CHECK_OPTIONAL_RELATIONAL_FALSE_TRUE(<, >=, optn, optn_);
		CHECK_OPTIONAL_RELATIONAL_TRUE_TRUE(<, >=, optn, opt1);
		CHECK_OPTIONAL_RELATIONAL_TRUE_TRUE(<, >=, optn, opt2);

		CHECK_OPTIONAL_RELATIONAL_FALSE_FALSE(<, >=, opt1, null);
		CHECK_OPTIONAL_RELATIONAL_FALSE_TRUE(<, >=, opt1, v1);
		CHECK_OPTIONAL_RELATIONAL_TRUE_TRUE(<, >=, opt1, v2);
		CHECK_OPTIONAL_RELATIONAL_FALSE_FALSE(<, >=, opt1, optn);
		CHECK_OPTIONAL_RELATIONAL_FALSE_TRUE(<, >=, opt1, opt1_);
		CHECK_OPTIONAL_RELATIONAL_TRUE_TRUE(<, >=, opt1, opt2);

		CHECK_OPTIONAL_RELATIONAL_FALSE_FALSE(<, >=, opt2, null);
		CHECK_OPTIONAL_RELATIONAL_FALSE_FALSE(<, >=, opt2, v1);
		CHECK_OPTIONAL_RELATIONAL_FALSE_TRUE(<, >=, opt2, v2);
		CHECK_OPTIONAL_RELATIONAL_FALSE_FALSE(<, >=, opt2, optn);
		CHECK_OPTIONAL_RELATIONAL_FALSE_FALSE(<, >=, opt2, opt1);
	}

	{
		CHECK_OPTIONAL_RELATIONAL_FALSE_TRUE(>, <=, optn, null);
		CHECK_OPTIONAL_RELATIONAL_FALSE_FALSE(>, <=, optn, v1);
		CHECK_OPTIONAL_RELATIONAL_FALSE_FALSE(>, <=, optn, v2);
		CHECK_OPTIONAL_RELATIONAL_FALSE_TRUE(>, <=, optn, optn_);
		CHECK_OPTIONAL_RELATIONAL_FALSE_FALSE(>, <=, optn, opt1);
		CHECK_OPTIONAL_RELATIONAL_FALSE_FALSE(>, <=, optn, opt2);

		CHECK_OPTIONAL_RELATIONAL_TRUE_TRUE(>, <=, opt1, null);
		CHECK_OPTIONAL_RELATIONAL_FALSE_TRUE(>, <=, opt1, v1);
		CHECK_OPTIONAL_RELATIONAL_FALSE_FALSE(>, <=, opt1, v2);
		CHECK_OPTIONAL_RELATIONAL_TRUE_TRUE(>, <=, opt1, optn);
		CHECK_OPTIONAL_RELATIONAL_FALSE_TRUE(>, <=, opt1, opt1_);
		CHECK_OPTIONAL_RELATIONAL_FALSE_FALSE(>, <=, opt1, opt2);

		CHECK_OPTIONAL_RELATIONAL_TRUE_TRUE(>, <=, opt2, null);
		CHECK_OPTIONAL_RELATIONAL_TRUE_TRUE(>, <=, opt2, v1);
		CHECK_OPTIONAL_RELATIONAL_FALSE_TRUE(>, <=, opt2, v2);
		CHECK_OPTIONAL_RELATIONAL_TRUE_TRUE(>, <=, opt2, optn);
		CHECK_OPTIONAL_RELATIONAL_TRUE_TRUE(>, <=, opt2, opt1);
	}

	{
		CHECK_OPTIONAL_RELATIONAL_TRUE_FALSE(<=, >, optn, null);
		CHECK_OPTIONAL_RELATIONAL_TRUE_TRUE(<=, >, optn, v1);
		CHECK_OPTIONAL_RELATIONAL_TRUE_TRUE(<=, >, optn, v2);
		CHECK_OPTIONAL_RELATIONAL_TRUE_FALSE(<=, >, optn, optn_);
		CHECK_OPTIONAL_RELATIONAL_TRUE_TRUE(<=, >, optn, opt1);
		CHECK_OPTIONAL_RELATIONAL_TRUE_TRUE(<=, >, optn, opt2);

		CHECK_OPTIONAL_RELATIONAL_FALSE_FALSE(<=, >, opt1, null);
		CHECK_OPTIONAL_RELATIONAL_TRUE_FALSE(<=, >, opt1, v1);
		CHECK_OPTIONAL_RELATIONAL_TRUE_TRUE(<=, >, opt1, v2);
		CHECK_OPTIONAL_RELATIONAL_FALSE_FALSE(<=, >, opt1, optn);
		CHECK_OPTIONAL_RELATIONAL_TRUE_FALSE(<=, >, opt1, opt1_);
		CHECK_OPTIONAL_RELATIONAL_TRUE_TRUE(<=, >, opt1, opt2);

		CHECK_OPTIONAL_RELATIONAL_FALSE_FALSE(<=, >, opt2, null);
		CHECK_OPTIONAL_RELATIONAL_FALSE_FALSE(<=, >, opt2, v1);
		CHECK_OPTIONAL_RELATIONAL_TRUE_FALSE(<=, >, opt2, v2);
		CHECK_OPTIONAL_RELATIONAL_FALSE_FALSE(<=, >, opt2, optn);
		CHECK_OPTIONAL_RELATIONAL_FALSE_FALSE(<=, >, opt2, opt1);
	}

	{
		CHECK_OPTIONAL_RELATIONAL_TRUE_FALSE(>=, <, optn, null);
		CHECK_OPTIONAL_RELATIONAL_FALSE_FALSE(>=, <, optn, v1);
		CHECK_OPTIONAL_RELATIONAL_FALSE_FALSE(>=, <, optn, v2);
		CHECK_OPTIONAL_RELATIONAL_TRUE_FALSE(>=, <, optn, optn_);
		CHECK_OPTIONAL_RELATIONAL_FALSE_FALSE(>=, <, optn, opt1);
		CHECK_OPTIONAL_RELATIONAL_FALSE_FALSE(>=, <, optn, opt2);

		CHECK_OPTIONAL_RELATIONAL_TRUE_TRUE(>=, <, opt1, null);
		CHECK_OPTIONAL_RELATIONAL_TRUE_FALSE(>=, <, opt1, v1);
		CHECK_OPTIONAL_RELATIONAL_FALSE_FALSE(>=, <, opt1, v2);
		CHECK_OPTIONAL_RELATIONAL_TRUE_TRUE(>=, <, opt1, optn);
		CHECK_OPTIONAL_RELATIONAL_TRUE_FALSE(>=, <, opt1, opt1_);
		CHECK_OPTIONAL_RELATIONAL_FALSE_FALSE(>=, <, opt1, opt2);

		CHECK_OPTIONAL_RELATIONAL_TRUE_TRUE(>=, <, opt2, null);
		CHECK_OPTIONAL_RELATIONAL_TRUE_TRUE(>=, <, opt2, v1);
		CHECK_OPTIONAL_RELATIONAL_TRUE_FALSE(>=, <, opt2, v2);
		CHECK_OPTIONAL_RELATIONAL_TRUE_TRUE(>=, <, opt2, optn);
		CHECK_OPTIONAL_RELATIONAL_TRUE_TRUE(>=, <, opt2, opt1);
	}
}


#define CHECK_OPTIONAL_COMPARE_NON_COMMUTATIVE(Comparer, Relation, Lhs, Rhs) \
		ASSERT_##Relation(Comparer(Lhs, Rhs), 0);

#define CHECK_OPTIONAL_COMPARE(Comparer, Relation, OppositeRelation, Lhs, Rhs) \
		do { \
			ASSERT_##Relation(Comparer(Lhs, Rhs), 0); \
			ASSERT_##OppositeRelation(Comparer(Rhs, Lhs), 0); \
		} while (false)

#define CHECK_OPTIONAL_COMPARE_TRUE_TRUE(Comparer, Lhs, Rhs) \
		do { \
			ASSERT_TRUE(Comparer(Lhs, Rhs)); \
			ASSERT_TRUE(Comparer(Rhs, Lhs)); \
		} while (false)

#define CHECK_OPTIONAL_COMPARE_FALSE_FALSE(Comparer, Lhs, Rhs) \
		do { \
			ASSERT_FALSE(Comparer(Lhs, Rhs)); \
			ASSERT_FALSE(Comparer(Rhs, Lhs)); \
		} while (false)

#define CHECK_OPTIONAL_COMPARE_TRUE_FALSE(Comparer, Lhs, Rhs) \
		do { \
			ASSERT_TRUE(Comparer(Lhs, Rhs)); \
			ASSERT_FALSE(Comparer(Rhs, Lhs)); \
		} while (false)

#define CHECK_OPTIONAL_COMPARE_FALSE_TRUE(Comparer, Lhs, Rhs) \
		do { \
			ASSERT_FALSE(Comparer(Lhs, Rhs)); \
			ASSERT_TRUE(Comparer(Rhs, Lhs)); \
		} while (false)

TEST(OptionalTest, Compare)
{
	const int v1 = 1;
	const int v2 = 2;
	const optional<int> optn;
	const optional<int> optn_;
	const optional<int> opt1(1);
	const optional<int> opt1_(1);
	const optional<int> opt2(2);

	{
		ASSERT_EQ(optn.Compare(null), 0);
		ASSERT_EQ(optn.Compare(v1), -1);
		ASSERT_EQ(optn.Compare(v2), -1);
		ASSERT_EQ(optn.Compare(optn_), 0);
		ASSERT_EQ(optn.Compare(opt1), -1);
		ASSERT_EQ(optn.Compare(opt2), -1);

		ASSERT_EQ(opt1.Compare(null), 1);
		ASSERT_EQ(opt1.Compare(v1), 0);
		ASSERT_EQ(opt1.Compare(v2), -1);
		ASSERT_EQ(opt1.Compare(optn), 1);
		ASSERT_EQ(opt1.Compare(opt1_), 0);
		ASSERT_EQ(opt1.Compare(opt2), -1);

		ASSERT_EQ(opt2.Compare(null), 1);
		ASSERT_EQ(opt2.Compare(v1), 1);
		ASSERT_EQ(opt2.Compare(v2), 0);
		ASSERT_EQ(opt2.Compare(optn), 1);
		ASSERT_EQ(opt2.Compare(opt1), 1);
	}

	{
		const auto cmp = comparers::Cmp();

		CHECK_OPTIONAL_COMPARE_NON_COMMUTATIVE(cmp, EQ, optn, null);
		CHECK_OPTIONAL_COMPARE_NON_COMMUTATIVE(cmp, LT, optn, v1);
		CHECK_OPTIONAL_COMPARE_NON_COMMUTATIVE(cmp, LT, optn, v2);
		CHECK_OPTIONAL_COMPARE(cmp, EQ, EQ, optn, optn_);
		CHECK_OPTIONAL_COMPARE(cmp, LT, GT, optn, opt1);
		CHECK_OPTIONAL_COMPARE(cmp, LT, GT, optn, opt2);

		CHECK_OPTIONAL_COMPARE_NON_COMMUTATIVE(cmp, GT, opt1, null);
		CHECK_OPTIONAL_COMPARE_NON_COMMUTATIVE(cmp, EQ, opt1, v1);
		CHECK_OPTIONAL_COMPARE_NON_COMMUTATIVE(cmp, LT, opt1, v2);
		CHECK_OPTIONAL_COMPARE(cmp, GT, LT, opt1, optn);
		CHECK_OPTIONAL_COMPARE(cmp, EQ, EQ, opt1, opt1_);
		CHECK_OPTIONAL_COMPARE(cmp, LT, GT, opt1, opt2);

		CHECK_OPTIONAL_COMPARE_NON_COMMUTATIVE(cmp, GT, opt2, null);
		CHECK_OPTIONAL_COMPARE_NON_COMMUTATIVE(cmp, GT, opt2, v1);
		CHECK_OPTIONAL_COMPARE_NON_COMMUTATIVE(cmp, EQ, opt2, v2);
		CHECK_OPTIONAL_COMPARE(cmp, GT, LT, opt2, optn);
		CHECK_OPTIONAL_COMPARE(cmp, GT, LT, opt2, opt1);
	}

	{
		const auto cmp = OptionalCmp<comparers::Cmp>();

		CHECK_OPTIONAL_COMPARE(cmp, EQ, EQ, optn, null);
		CHECK_OPTIONAL_COMPARE(cmp, LT, GT, optn, v1);
		CHECK_OPTIONAL_COMPARE(cmp, LT, GT, optn, v2);
		CHECK_OPTIONAL_COMPARE(cmp, EQ, EQ, optn, optn_);
		CHECK_OPTIONAL_COMPARE(cmp, LT, GT, optn, opt1);
		CHECK_OPTIONAL_COMPARE(cmp, LT, GT, optn, opt2);

		CHECK_OPTIONAL_COMPARE(cmp, GT, LT, opt1, null);
		CHECK_OPTIONAL_COMPARE(cmp, EQ, EQ, opt1, v1);
		CHECK_OPTIONAL_COMPARE(cmp, LT, GT, opt1, v2);
		CHECK_OPTIONAL_COMPARE(cmp, GT, LT, opt1, optn);
		CHECK_OPTIONAL_COMPARE(cmp, EQ, EQ, opt1, opt1_);
		CHECK_OPTIONAL_COMPARE(cmp, LT, GT, opt1, opt2);

		CHECK_OPTIONAL_COMPARE(cmp, GT, LT, opt2, null);
		CHECK_OPTIONAL_COMPARE(cmp, GT, LT, opt2, v1);
		CHECK_OPTIONAL_COMPARE(cmp, EQ, EQ, opt2, v2);
		CHECK_OPTIONAL_COMPARE(cmp, GT, LT, opt2, optn);
		CHECK_OPTIONAL_COMPARE(cmp, GT, LT, opt2, opt1);
	}

	{
		const auto less = OptionalLess<comparers::Cmp>();

		CHECK_OPTIONAL_COMPARE_FALSE_FALSE(less, optn, null);
		CHECK_OPTIONAL_COMPARE_TRUE_FALSE(less, optn, v1);
		CHECK_OPTIONAL_COMPARE_TRUE_FALSE(less, optn, v2);
		CHECK_OPTIONAL_COMPARE_FALSE_FALSE(less, optn, optn_);
		CHECK_OPTIONAL_COMPARE_TRUE_FALSE(less, optn, opt1);
		CHECK_OPTIONAL_COMPARE_TRUE_FALSE(less, optn, opt2);

		CHECK_OPTIONAL_COMPARE_FALSE_TRUE(less, opt1, null);
		CHECK_OPTIONAL_COMPARE_FALSE_FALSE(less, opt1, v1);
		CHECK_OPTIONAL_COMPARE_TRUE_FALSE(less, opt1, v2);
		CHECK_OPTIONAL_COMPARE_FALSE_TRUE(less, opt1, optn);
		CHECK_OPTIONAL_COMPARE_FALSE_FALSE(less, opt1, opt1_);
		CHECK_OPTIONAL_COMPARE_TRUE_FALSE(less, opt1, opt2);

		CHECK_OPTIONAL_COMPARE_FALSE_TRUE(less, opt2, null);
		CHECK_OPTIONAL_COMPARE_FALSE_TRUE(less, opt2, v1);
		CHECK_OPTIONAL_COMPARE_FALSE_FALSE(less, opt2, v2);
		CHECK_OPTIONAL_COMPARE_FALSE_TRUE(less, opt2, optn);
		CHECK_OPTIONAL_COMPARE_FALSE_TRUE(less, opt2, opt1);
	}

	{
		const auto equals = OptionalEquals<comparers::Cmp>();

		CHECK_OPTIONAL_COMPARE_TRUE_TRUE(equals, optn, null);
		CHECK_OPTIONAL_COMPARE_FALSE_FALSE(equals, optn, v1);
		CHECK_OPTIONAL_COMPARE_FALSE_FALSE(equals, optn, v2);
		CHECK_OPTIONAL_COMPARE_TRUE_TRUE(equals, optn, optn_);
		CHECK_OPTIONAL_COMPARE_FALSE_FALSE(equals, optn, opt1);
		CHECK_OPTIONAL_COMPARE_FALSE_FALSE(equals, optn, opt2);

		CHECK_OPTIONAL_COMPARE_FALSE_FALSE(equals, opt1, null);
		CHECK_OPTIONAL_COMPARE_TRUE_TRUE(equals, opt1, v1);
		CHECK_OPTIONAL_COMPARE_FALSE_FALSE(equals, opt1, v2);
		CHECK_OPTIONAL_COMPARE_FALSE_FALSE(equals, opt1, optn);
		CHECK_OPTIONAL_COMPARE_TRUE_TRUE(equals, opt1, opt1_);
		CHECK_OPTIONAL_COMPARE_FALSE_FALSE(equals, opt1, opt2);

		CHECK_OPTIONAL_COMPARE_FALSE_FALSE(equals, opt2, null);
		CHECK_OPTIONAL_COMPARE_FALSE_FALSE(equals, opt2, v1);
		CHECK_OPTIONAL_COMPARE_TRUE_TRUE(equals, opt2, v2);
		CHECK_OPTIONAL_COMPARE_FALSE_FALSE(equals, opt2, optn);
		CHECK_OPTIONAL_COMPARE_FALSE_FALSE(equals, opt2, opt1);
	}

	{
		const auto greater = OptionalGreater<comparers::Cmp>();

		CHECK_OPTIONAL_COMPARE_FALSE_FALSE(greater, optn, null);
		CHECK_OPTIONAL_COMPARE_FALSE_TRUE(greater, optn, v1);
		CHECK_OPTIONAL_COMPARE_FALSE_TRUE(greater, optn, v2);
		CHECK_OPTIONAL_COMPARE_FALSE_FALSE(greater, optn, optn_);
		CHECK_OPTIONAL_COMPARE_FALSE_TRUE(greater, optn, opt1);
		CHECK_OPTIONAL_COMPARE_FALSE_TRUE(greater, optn, opt2);

		CHECK_OPTIONAL_COMPARE_TRUE_FALSE(greater, opt1, null);
		CHECK_OPTIONAL_COMPARE_FALSE_FALSE(greater, opt1, v1);
		CHECK_OPTIONAL_COMPARE_FALSE_TRUE(greater, opt1, v2);
		CHECK_OPTIONAL_COMPARE_TRUE_FALSE(greater, opt1, optn);
		CHECK_OPTIONAL_COMPARE_FALSE_FALSE(greater, opt1, opt1_);
		CHECK_OPTIONAL_COMPARE_FALSE_TRUE(greater, opt1, opt2);

		CHECK_OPTIONAL_COMPARE_TRUE_FALSE(greater, opt2, null);
		CHECK_OPTIONAL_COMPARE_TRUE_FALSE(greater, opt2, v1);
		CHECK_OPTIONAL_COMPARE_FALSE_FALSE(greater, opt2, v2);
		CHECK_OPTIONAL_COMPARE_TRUE_FALSE(greater, opt2, optn);
		CHECK_OPTIONAL_COMPARE_TRUE_FALSE(greater, opt2, opt1);
	}
}
