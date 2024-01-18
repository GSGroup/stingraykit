// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#ifndef UNITTESTS_ENUMERABLEMATCHER_H
#define UNITTESTS_ENUMERABLEMATCHER_H

#include <stingraykit/collection/StlEnumeratorAdapter.h>

#include <gmock/gmock-matchers.h>

namespace stingray
{

	namespace Detail
	{

		template < typename InnerMatcher >
		class EnumerableMatcher
		{
		private:
			template < typename EnumerablePtr >
			class Impl : public testing::MatcherInterface<EnumerablePtr>
			{
				using Container = std::vector<typename Decay<EnumerablePtr>::ValueT::ValueType::ItemType>;

			private:
				const testing::Matcher<const Container&>	_matcher;

			public:
				explicit Impl(const InnerMatcher& matcher)
					:	_matcher(testing::MatcherCast<const Container&>(matcher))
				{ }

				void DescribeTo(std::ostream* os) const override
				{
					*os << "points to an enumerable that ";
					_matcher.DescribeTo(os);
				}

				void DescribeNegationTo(std::ostream* os) const override
				{
					*os << "does not point to an enumerable that ";
					_matcher.DescribeTo(os);
				}

				bool MatchAndExplain(EnumerablePtr enumerable, testing::MatchResultListener* listener) const override
				{
					if (!enumerable)
						return false;

					const auto iterableEnumerable = IterableEnumerable(enumerable);
					const Container container(iterableEnumerable.begin(), iterableEnumerable.end());
					return MatchPrintAndExplain(container, _matcher, listener);
				}
			};

		private:
			const InnerMatcher	_matcher;

		public:
			explicit EnumerableMatcher(const InnerMatcher& matcher)
				:	_matcher(matcher)
			{ }

			template < typename EnumerablePtr >
			operator testing::Matcher<EnumerablePtr>() const
			{ return testing::Matcher<EnumerablePtr>(new Impl<const EnumerablePtr&>(_matcher)); }
		};

	}


	template < typename InnerMatcher >
	auto MatchEnumerable(const InnerMatcher& matcher)
	{ return Detail::EnumerableMatcher<InnerMatcher>(matcher); }

}

#endif
