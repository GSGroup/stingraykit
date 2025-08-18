#ifndef UNITTESTS_OPTIONALMATCHER_H
#define UNITTESTS_OPTIONALMATCHER_H

// Copyright (c) 2011 - 2025, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/optional.h>

#include <gmock/gmock-matchers.h>

namespace stingray
{

	namespace Detail
	{

		template < typename ValueMatcher >
		class OptionalMatcher
		{
		private:
			template < typename Optional >
			class Impl : public testing::MatcherInterface<Optional>
			{
				using ValueType = typename Decay<Optional>::ValueT::ValueT;

			private:
				const testing::Matcher<ValueType>	_matcher;

			public:
				explicit Impl(const ValueMatcher& matcher)
					: _matcher(testing::MatcherCast<ValueType>(matcher))
				{ }

				void DescribeTo(std::ostream* os) const override
				{
					*os << "value ";
					_matcher.DescribeTo(os);
				}

				void DescribeNegationTo(std::ostream* os) const override
				{
					*os << "value ";
					_matcher.DescribeNegationTo(os);
				}

				bool MatchAndExplain(Optional optional, testing::MatchResultListener* listener) const override
				{
					if (!optional)
					{
						*listener << "which is not engaged";
						return false;
					}

					return MatchPrintAndExplain(*optional, _matcher, listener);
				}
			};

		private:
			const ValueMatcher		_matcher;

		public:
			explicit OptionalMatcher(const ValueMatcher& matcher)
				: _matcher(matcher)
			{ }

			template < typename Optional >
			operator testing::Matcher<Optional>() const
			{ return testing::Matcher<Optional>(new Impl<const Optional&>(_matcher)); }
		};

	}


	template < typename ValueMatcher >
	Detail::OptionalMatcher<ValueMatcher> MatchOptional(const ValueMatcher& matcher)
	{ return Detail::OptionalMatcher<ValueMatcher>(matcher); }

}

#endif
