// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/string/RegexUtils.h>

#include <stingraykit/string/StringUtils.h>
#include <stingraykit/math.h>

namespace stingray
{

	namespace
	{

		constexpr u64 Radix = 10;
		constexpr u64 MaxValue = std::numeric_limits<u64>::max();
		constexpr u64 MaxBase = MaxValue / Radix;

		void GenerateNumericRangeRegexStageOne(std::vector<std::string>& result, size_t minDigits, u64 start, u64 end);

		void GenerateNumericRangeRegexStageTwo(std::vector<std::string>& result, size_t minDigits, u64 base, u64 start, u64 end)
		{
			STINGRAYKIT_CHECK(minDigits > 0, ArgumentException("minDigits"));
			STINGRAYKIT_CHECK(IsPowerOfNumber<Radix>(base), ArgumentException("base", base));

			const size_t digits = ToString(base).size();
			STINGRAYKIT_CHECK(minDigits >= digits, ArgumentException("(minDigits, base)", MakeTuple(minDigits, base)));

			const u64 normalizedBase = base == 1 ? 0 : base;
			const bool hasNextBase = base <= MaxBase;
			const u64 nextBase = hasNextBase ? base * Radix : MaxValue;
			STINGRAYKIT_CHECK((start == 0 || normalizedBase <= start) && (hasNextBase ? start < nextBase : start <= nextBase), ArgumentException("(base, start)", MakeTuple(base, start)));
			STINGRAYKIT_CHECK(normalizedBase <= end && (hasNextBase ? end < nextBase : end <= nextBase), ArgumentException("(base, end)", MakeTuple(base, end)));

			STINGRAYKIT_CHECK(start <= end, ArgumentException("(start, end)", MakeTuple(start, end)));

			if (start == end)
			{
				result.push_back(RightJustify(ToString(start), minDigits, '0'));
				return;
			}

			const std::string zeroesPrefix(minDigits - digits, '0');

			const u64 startQuotient = start / base;
			const u64 endQuotient = end / base;

			const u64 startRemainder = start % base;
			const u64 endRemainder = end % base;

			if (startQuotient == endQuotient)
			{
				if (base == 1)
					result.push_back(StringBuilder() % zeroesPrefix % startQuotient);
				else
				{
					std::vector<std::string> tailRegexes;
					GenerateNumericRangeRegexStageOne(tailRegexes, digits - 1, startRemainder, endRemainder);

					for (const auto& tailRegex : tailRegexes)
						result.push_back(StringBuilder() % zeroesPrefix % startQuotient % tailRegex);
				}

				return;
			}

			const bool isStartAligned = startRemainder == 0;
			const bool isEndAligned = endRemainder == base - 1;

			if (!isStartAligned)
			{
				STINGRAYKIT_CHECK(base > 1, LogicException(StringBuilder() % "Failed to generate regex for " % MakeTuple(minDigits, base, start, end)));

				std::vector<std::string> tailRegexes;
				GenerateNumericRangeRegexStageOne(tailRegexes, digits - 1, startRemainder, base - 1);

				for (const auto& tailRegex : tailRegexes)
					result.push_back(StringBuilder() % zeroesPrefix % startQuotient % tailRegex);
			}

			const u64 firstValue = isStartAligned ? start : start - startRemainder + base;
			const u64 lastValue = isEndAligned ? end : end - endRemainder - 1;
			STINGRAYKIT_CHECK(lastValue % base == base - 1, LogicException(StringBuilder() % "Failed to generate regex for " % MakeTuple(minDigits, base, start, end)));

			const u64 firstQuotient = firstValue / base;
			const u64 lastQuotient = lastValue / base;

			if (firstQuotient <= lastQuotient)
			{
				const std::string quotientRegex = firstQuotient == lastQuotient ? ToString(firstQuotient) :
						StringBuilder() % "[" % firstQuotient % (firstQuotient + 1 == lastQuotient ? "" : "-") % lastQuotient % "]";

				if (base == 1)
					result.push_back(StringBuilder() % zeroesPrefix % quotientRegex);
				else
				{
					std::vector<std::string> tailRegexes;
					GenerateNumericRangeRegexStageOne(tailRegexes, digits - 1, firstValue % base, lastValue % base);

					STINGRAYKIT_CHECK(tailRegexes.size() == 1, LogicException(StringBuilder() % "Failed to generate regex for " % MakeTuple(minDigits, base, start, end)));
					result.push_back(StringBuilder() % zeroesPrefix % quotientRegex % tailRegexes.front());
				}
			}

			if (!isEndAligned)
			{
				STINGRAYKIT_CHECK(base > 1, LogicException(StringBuilder() % "Failed to generate regex for " % MakeTuple(minDigits, base, start, end)));

				std::vector<std::string> tailRegexes;
				GenerateNumericRangeRegexStageOne(tailRegexes, digits - 1, 0, endRemainder);

				for (const auto& tailRegex : tailRegexes)
					result.push_back(StringBuilder() % zeroesPrefix % endQuotient % tailRegex);
			}
		}

		void GenerateNumericRangeRegexStageOne(std::vector<std::string>& result, size_t minDigits, u64 start, u64 end)
		{
			STINGRAYKIT_CHECK(start <= end, ArgumentException("(start, end)", MakeTuple(start, end)));

			const size_t prevResultSize = result.size();

			bool firstBase = true;
			size_t digits = 1;
			for (u64 base = 1, normalizedBase = 0, nextBase = base * Radix;
					normalizedBase <= end && base < MaxValue;
					base = nextBase, normalizedBase = base, nextBase = nextBase <= MaxBase ? nextBase * Radix : MaxValue, ++digits)
			{
				if (nextBase != MaxValue && start >= nextBase)
					continue;

				if (digits < minDigits && start == 0 && end >= nextBase)
					continue;

				GenerateNumericRangeRegexStageTwo(
						result,
						std::max(minDigits, digits),
						base,
						firstBase && start == 0 ? start : std::max(normalizedBase, start),
						std::min(nextBase == MaxValue ? nextBase : nextBase - 1, end));

				firstBase = false;
			}

			STINGRAYKIT_CHECK(result.size() > prevResultSize, LogicException(StringBuilder() % "Failed to generate regex for " % MakeTuple(minDigits, start, end)));
		}

	}


	std::string GenerateNumericRangeRegex(u64 start, u64 end, bool match)
	{
		STINGRAYKIT_CHECK(start <= end, ArgumentException("(start, end)", MakeTuple(start, end)));

		std::vector<std::string> regexes;
		GenerateNumericRangeRegexStageOne(regexes, 0, start, end);

		if (regexes.size() == 1 && !match)
			return regexes.front();

		StringJoiner result("|", match ? "(" : "(?:", ")");

		for (auto rit = regexes.rbegin(); rit != regexes.rend(); ++rit)
			result % *rit;

		return result;
	}

}
