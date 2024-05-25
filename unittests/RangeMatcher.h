#ifndef UNITTESTS_RANGEMATCHER_H
#define UNITTESTS_RANGEMATCHER_H

// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/collection/Range.h>

#include <gmock/gmock-matchers.h>

namespace stingray
{

	namespace Detail
	{

		template < typename InnerMatcher >
		class RangeMatcher
		{
		private:
			template < typename Range_ >
			class Impl : public testing::MatcherInterface<Range_>
			{
				using Container = std::vector<typename Decay<typename Decay<Range_>::ValueT::ValueType>::ValueT>;

			private:
				const testing::Matcher<const Container&>	_matcher;

			public:
				explicit Impl(const InnerMatcher& matcher)
					:	_matcher(testing::MatcherCast<const Container&>(matcher))
				{ }

				void DescribeTo(std::ostream* os) const override
				{
					*os << "a range that ";
					_matcher.DescribeTo(os);
				}

				void DescribeNegationTo(std::ostream* os) const override
				{
					*os << "not a range that ";
					_matcher.DescribeTo(os);
				}

				bool MatchAndExplain(Range_ range, testing::MatchResultListener* listener) const override
				{
					const auto iterableRange = IterableRange(range);
					const Container container(iterableRange.begin(), iterableRange.end());
					return MatchPrintAndExplain(container, _matcher, listener);
				}
			};

		private:
			const InnerMatcher	_matcher;

		public:
			explicit RangeMatcher(const InnerMatcher& matcher)
				:	_matcher(matcher)
			{ }

			template < typename Range_ >
			operator testing::Matcher<Range_>() const
			{ return testing::Matcher<Range_>(new Impl<const Range_&>(_matcher)); }
		};

	}


	template < typename InnerMatcher >
	auto MatchRange(const InnerMatcher& matcher)
	{ return Detail::RangeMatcher<InnerMatcher>(matcher); }

}

#endif
