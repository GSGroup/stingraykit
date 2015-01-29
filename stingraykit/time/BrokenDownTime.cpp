#include <stingraykit/time/BrokenDownTime.h>

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


	std::string BrokenDownTime::ToString(const std::string& format) const
	{
		std::string result = format.empty() ? "dd/MM/YYYY hh:mm:ss.lll" : format;

		// TODO: reimplement
		ReplaceAll(result, "dd", RightJustify(stingray::ToString(MonthDay), 2, '0'));
		ReplaceAll(result, "MM", RightJustify(stingray::ToString(Month), 2, '0'));
		ReplaceAll(result, "Y", "y");
		ReplaceAll(result, "yyyy", RightJustify(stingray::ToString(Year), 4, '0'));
		ReplaceAll(result, "hh",  RightJustify(stingray::ToString(Hours), 2, '0'));
		ReplaceAll(result, "mm", RightJustify(stingray::ToString(Minutes), 2, '0'));
		ReplaceAll(result, "ss", RightJustify(stingray::ToString(Seconds), 2, '0'));
		ReplaceAll(result, "lll", RightJustify(stingray::ToString(Milliseconds), 3, '0'));

		return result;
	}

}
