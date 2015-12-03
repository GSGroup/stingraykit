// Copyright (c) 2011 - 2015, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/time/BrokenDownTime.h>

#include <stingraykit/function/bind.h>
#include <stingraykit/string/AhoCorasick.h>
#include <stingraykit/string/StringUtils.h>
#include <stingraykit/string/ToString.h>


namespace stingray
{

	BrokenDownTime BrokenDownTime::GetDayStart()
	{ return BrokenDownTime(0, 0, 0, 0, WeekDay, MonthDay, Month, YearDay, Year); }


	int BrokenDownTime::GetMaxDaysInMonth() const
	{
		static int daysPerMonths[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
		if (Month == 2 && Year % 4 == 0 && (Year % 100 != 0 || Year % 400 == 0))
			return 29;
		else
			return daysPerMonths[Month - 1];
	}


	int BrokenDownTime::GetMaxDaysInMonth(int year, int month)
	{
		BrokenDownTime t;
		t.Year = year;
		t.Month = month;
		return t.GetMaxDaysInMonth();
	}

	class BrokenDownTime::FormatMatcher
	{
		AhoCorasick _aho;

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

		static void OnMatch(const BrokenDownTime & bdt, string_ostream & stream, size_t & lastPosition, const std::string &format, const std::string &pattern, size_t patternIndex, size_t offset)
		{
			stream.write(format.data() + lastPosition, offset - lastPosition);
			switch(patternIndex)
			{
				case 0: stream << RightJustify(stingray::ToString(bdt.MonthDay), 2, '0'); break;
				case 1: stream << RightJustify(stingray::ToString(bdt.Month), 2, '0'); break;
				case 2:
				case 3: stream << RightJustify(stingray::ToString(bdt.Year), 4, '0'); break;
				case 4: stream << RightJustify(stingray::ToString(bdt.Hours), 2, '0'); break;
				case 5: stream << RightJustify(stingray::ToString(bdt.Minutes), 2, '0'); break;
				case 6: stream << RightJustify(stingray::ToString(bdt.Seconds), 2, '0'); break;
				case 7: stream << RightJustify(stingray::ToString(bdt.Milliseconds), 3, '0'); break;
			}
			lastPosition = offset + pattern.size();
		}

		void Search(const std::string & text, const AhoCorasick::CallbackType & callback) const
		{ _aho.Search(text, callback); }
	};

	BrokenDownTime::FormatMatcher BrokenDownTime::s_formatMatcher;

	std::string BrokenDownTime::ToString(const std::string & format) const
	{
		if (format.empty())
		{
#if 0
			return ToString("dd/MM/YYYY hh:mm:ss.lll");
#else
			string_ostream stream;
			stream << RightJustify(stingray::ToString(MonthDay), 2, '0') << '/'; //fixme: use string_stream for justification
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
		size_t last = 0;

		AhoCorasick::CallbackType callback = bind(&FormatMatcher::OnMatch, ref(*this), ref(stream), ref(last), ref(format), _1, _2, _3);
		s_formatMatcher.Search(format, callback);
		stream.write(format.data() + last, format.size() - last);

		return stream.str();
	}

}
