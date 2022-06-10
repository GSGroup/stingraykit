#ifndef UNITTESTS_ENUMERABLEMATCHER_H
#define UNITTESTS_ENUMERABLEMATCHER_H

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
