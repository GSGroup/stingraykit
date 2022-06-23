#include <stingraykit/string/TranslatedStringUtils.h>

#include <gtest/gtest.h>

using namespace stingray;

TEST(TranslatedStringUtilsTest, Format)
{
	{
		const TranslatedString fmt = TranslatedString::Builder().AddTranslation(LangCode::Any, "Format %1%, %2%, %3%");
		const TranslatedString res = TranslatedStringFormat(fmt, "one", 2, TranslatedString::Builder().AddTranslation(LangCode::Any, "three").Get());
		const TranslatedString exp = TranslatedString::Builder().AddTranslation(LangCode::Any, "Format one, 2, three");
		ASSERT_EQ(res, exp);
	}

	{
		const TranslatedString fmt = TranslatedString::Builder()
				.AddTranslation(LangCode::Eng(), "Format %1%, %2%, %3%")
				.AddTranslation(LangCode::Rus(), "Формат %1%, %2%, %3%");
		const TranslatedString res = TranslatedStringFormat(fmt, "one", 2, TranslatedString::Builder()
				.AddTranslation(LangCode::Eng(), "three")
				.AddTranslation(LangCode::Rus(), "три").Get());
		const TranslatedString exp = TranslatedString::Builder()
				.AddTranslation(LangCode::Eng(), "Format one, 2, three")
				.AddTranslation(LangCode::Rus(), "Формат one, 2, три");
		ASSERT_EQ(res, exp);
	}
}


TEST(TranslatedStringUtilsTest, Builder)
{
	{
		TranslatedStringBuilder b(LangCode::Any);
		b % TranslatedString::Builder().AddTranslation(LangCode::Any, "Start ").Get();
		b % 42;
		b % " finish";

		const TranslatedString exp = TranslatedString::Builder().AddTranslation(LangCode::Any, "Start 42 finish");
		ASSERT_EQ(b.Get(), exp);
	}

	{
		TranslatedStringBuilder b(LangCode::Eng(), LangCode::Rus());
		b % TranslatedString::Builder().AddTranslation(LangCode::Eng(), "Start ").AddTranslation(LangCode::Rus(), "Начало ").Get();
		b % 42;
		b % " finish";

		const TranslatedString exp = TranslatedString::Builder()
				.AddTranslation(LangCode::Eng(), "Start 42 finish")
				.AddTranslation(LangCode::Rus(), "Начало 42 finish");
		ASSERT_EQ(b.Get(), exp);
	}
}
