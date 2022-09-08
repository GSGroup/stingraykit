// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/time/BrokenDownTime.h>

#include <stingraykit/string/AhoCorasick.h>
#include <stingraykit/string/StringUtils.h>

namespace stingray
{

	namespace
	{

		struct ReplaceContext
		{
			const BrokenDownTime&	Bdt;
			string_ostream&			Stream;
			mutable size_t			LastPosition;
			const std::string&		Format;

			ReplaceContext(const BrokenDownTime& bdt, string_ostream& stream, const std::string& format)
				: Bdt(bdt), Stream(stream), LastPosition(0), Format(format)
			{ }

			void operator () (const std::string& pattern, size_t patternIndex, size_t offset) const
			{
				Stream << string_view(Format.data() + LastPosition, offset - LastPosition);

				switch (patternIndex)
				{
				case 0: Stream << RightJustify(stingray::ToString(Bdt.MonthDay), 2, '0'); break;
				case 1: Stream << RightJustify(stingray::ToString(Bdt.Month), 2, '0'); break;
				case 2:
				case 3: Stream << RightJustify(stingray::ToString(Bdt.Year), 4, '0'); break;
				case 4: Stream << RightJustify(stingray::ToString(Bdt.Hours), 2, '0'); break;
				case 5: Stream << RightJustify(stingray::ToString(Bdt.Minutes), 2, '0'); break;
				case 6: Stream << RightJustify(stingray::ToString(Bdt.Seconds), 2, '0'); break;
				case 7: Stream << RightJustify(stingray::ToString(Bdt.Milliseconds), 3, '0'); break;
				}

				LastPosition = offset + pattern.size();
			}
		};

	}


	class BrokenDownTime::FormatMatcher
	{
	private:
		AhoCorasick		_aho;

	public:
		FormatMatcher()
		{
			_aho.Add("dd");
			_aho.Add("MM");
			_aho.Add("YYYY");
			_aho.Add("yyyy");
			_aho.Add("hh");
			_aho.Add("mm");
			_aho.Add("ss");
			_aho.Add("lll");
			_aho.Build();
		}

		template < typename CallbackType >
		void Search(const std::string& text, const CallbackType& callback) const
		{ _aho.Search(text, callback); }
	};


	const BrokenDownTime::FormatMatcher BrokenDownTime::s_formatMatcher;


	BrokenDownTime BrokenDownTime::GetDayStart() const
	{ return BrokenDownTime(0, 0, 0, 0, WeekDay, MonthDay, Month, YearDay, Year); }


	int BrokenDownTime::GetMaxDaysInMonth() const
	{
		static const int daysPerMonths[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

		if (Month == 2 && Year % 4 == 0 && (Year % 100 != 0 || Year % 400 == 0))
			return 29;
		else
			return ArrayGet(daysPerMonths, Month - 1);
	}


	int BrokenDownTime::GetMaxDaysInMonth(int year, int month)
	{
		BrokenDownTime bdt;
		bdt.Year = year;
		bdt.Month = month;
		return bdt.GetMaxDaysInMonth();
	}


	std::string BrokenDownTime::ToString(const std::string& format) const
	{
		if (format.empty())
		{
#if 0
			return ToString("dd/MM/YYYY hh:mm:ss.lll");
#else
			string_ostream stream;
			stream << RightJustify(stingray::ToString(MonthDay), 2, '0') << '/';
			stream << RightJustify(stingray::ToString(Month), 2, '0') << '/';
			stream << RightJustify(stingray::ToString(Year), 4, '0') << ' ';
			stream << RightJustify(stingray::ToString(Hours), 2, '0') << ':';
			stream << RightJustify(stingray::ToString(Minutes), 2, '0') << ':';
			stream << RightJustify(stingray::ToString(Seconds), 2, '0') << '.';
			stream << RightJustify(stingray::ToString(Milliseconds), 3, '0');
			return stream.str();
#endif
		}

		string_ostream stream;
		ReplaceContext context(*this, stream, format);

		s_formatMatcher.Search(format, context);
		stream << string_view(format.data() + context.LastPosition, format.size() - context.LastPosition);

		return stream.str();
	}

}
