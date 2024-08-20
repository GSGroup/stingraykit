#ifndef UNITTESTS_VARIANTMATCHER_H
#define UNITTESTS_VARIANTMATCHER_H

// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/variant.h>

#include <gmock/gmock-matchers.h>

namespace stingray
{

	namespace Detail
	{

		template < typename ValueType >
		class VariantMatcher
		{
		private:
			const testing::Matcher<const ValueType&>	_matcher;

		public:
			explicit VariantMatcher(const testing::Matcher<const ValueType&> matcher)
				:	_matcher(matcher)
			{ }

			template < typename Variant >
			bool MatchAndExplain(Variant variant, testing::MatchResultListener* listener) const
			{
				if (!variant.template contains<ValueType>())
				{
					*listener << "whose value is not of type '" << TypeInfo(typeid(ValueType)).ToString() << "'";
					return false;
				}

				return MatchPrintAndExplain(variant_get<ValueType>(variant), _matcher, listener);
			}

			void DescribeTo(std::ostream* os) const
			{
				*os << "is a variant with value of type " << TypeInfo(typeid(ValueType)).ToString() << " and the value ";
				_matcher.DescribeTo(os);
			}

			void DescribeNegationTo(std::ostream* os) const
			{
				*os << "is a variant with value of type other than " << TypeInfo(typeid(ValueType)).ToString() << " or the value ";
				_matcher.DescribeNegationTo(os);
			}
		};

	}

	template < typename ValueType >
	testing::PolymorphicMatcher<Detail::VariantMatcher<ValueType>> VariantWith(const testing::Matcher<const ValueType&>& matcher)
	{ return testing::MakePolymorphicMatcher(Detail::VariantMatcher<ValueType>(matcher)); }

}

#endif
