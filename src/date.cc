#include "date.hh"

#include <cstdlib>
#include <cstdio>

namespace koohar {

const char Date::months[12][4] = {"jan", "feb", "mar", "apr", "may", "jun", "jul", "aug", "sep", "oct", "nov", "dec"};
const unsigned short Date::day_per_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
const char Date::day_of_week[7][10] = {"monday", "tuesday", "wednesday", "thursday", "friday", "saturday", "sunday"};

Date::Date (const std::string& DateStr) : m_time(static_cast<time_t>(-1))
{
	size_t len = DateStr.length();
	if (len != 24 && len != 29 && (len < 29 || len > 33)) // gotta be some unsupported format
		return;
	size_t space = DateStr.find(' ');
	if (space == 3) // 'Sat '
		parse (DateStr.substr(space + 1), ASCTIME); // actually we do not need to know week day to calculate timestamp, so cut it out
	else if (space == 4) // 'Tue, '
		parse (DateStr.substr(space + 1), RFC1123);
	else if (space > 6 && space < 11) // 'Wednesday, '
		parse (DateStr.substr(space + 1), RFC850);
}

std::string Date::toString (DateFormat Format) const
{
	if (m_time == -1)
		return std::string();
	int t = static_cast<int>(m_time);
	int seconds = t % 60; t -= seconds; t /= 60;
	int minutes = t % 60; t -= minutes; t /= 60;
	int hours = t % 24; t -= hours; t /= 24;
	int day = t + 1;
	int year = 1970;
	int count = 2; // 1971'st year
	int day_number = day;
	while (day >= 365) {
		year++;
		day -= ((count % 4) == 0) ? 366 : 365;
		count++;
	}
	int month = dayToMonth(day, day, (year % 4) == 0);
	char ret[34];
	size_t week_day = (day_number + 2) % 7;
	if (Format == RFC1123) {
		sprintf(ret, "%s, %02d %s %d %02d:%02d:%02d GMT", std::string(day_of_week[week_day]).substr(0, 3).c_str(),
			day, months[month - 1], year, hours, minutes, seconds);
	} else if (Format == RFC850) {
		sprintf(ret, "%s, %02d-%s-%02d %02d:%02d:%02d GMT", std::string(day_of_week[week_day]).c_str(),
			day, months[month - 1], year % 100, hours, minutes, seconds);
	} else {
		sprintf(ret, "%s %s %02d %02d:%02d:%02d %04d", std::string(day_of_week[week_day]).substr(0, 3).c_str(),
			months[month - 1], day, hours, minutes, seconds, year);
	}
	return ret;
}

int Date::monthToNumber (const std::string& MonthStr) const
{
	if (MonthStr.length() != 3)
		return 0;
	for (unsigned short count = 0; count < 12; count++)
		if (!MonthStr.compare(months[count]))
			return static_cast<int>(count + 1);
	return 0;
}

std::string Date::numberToMonth (const int MonthNumber) const
{
	return months[MonthNumber];
}

int Date::monthToDay (const std::string& MonthStr) const
{
	unsigned short month = monthToNumber(MonthStr);
	unsigned short ret = 0;
	for (unsigned short count = 1; count < month; count++)
		ret += day_per_month[count - 1];
	return ret;
}

int Date::dayToMonth (const int DayNumber, int& Day, bool leap_year) const
{
	if (DayNumber > (leap_year ? 366 : 365))
		return 0;
	unsigned short month = 1;
	Day = DayNumber;
	while (Day >= day_per_month[month - 1]) {
		if (leap_year && month == 1)
			Day -= 1;
		Day -= day_per_month[month - 1];
		month++;
	};
	if (!Day)
		Day = 1;
	return month;
}

Date& Date::incDays (const size_t DayNumber)
{
	m_time += DayNumber * 24 * 60 * 60;
	return *this;
}

Date& Date::incHours (const size_t HourNumber)
{
	m_time += HourNumber * 60 * 60;
	return *this;
}

Date& Date::incMinutes (const size_t MinuteNumber)
{
	m_time += MinuteNumber * 60;
	return *this;
}

// private methods

void Date::parse (const std::string& DateStr, DateFormat Format)
{
	int day, month, year, hours, minutes, seconds;
	if (Format == RFC1123) {
		day = atoi(DateStr.substr(0, 2).c_str());
		month = monthToDay(DateStr.substr(3, 3));
		year = atoi(DateStr.substr(7, 4).c_str());
		hours = atoi(DateStr.substr(12, 2).c_str());
		minutes = atoi(DateStr.substr(15, 2).c_str());
		seconds = atoi(DateStr.substr(18, 2).c_str());
	} else if (Format == RFC850) {
		day = atoi(DateStr.substr(0, 2).c_str());
		month = monthToDay(DateStr.substr(3, 3));
		year = 2000 + atoi(DateStr.substr(7, 2).c_str()); // quite strange date format =/
		hours = atoi(DateStr.substr(10, 2).c_str());
		minutes = atoi(DateStr.substr(13, 2).c_str());
		seconds = atoi(DateStr.substr(16, 2).c_str());
	} else {
		month = monthToDay(DateStr.substr(0, 3).c_str());
		day = atoi(DateStr.substr(4, 2).c_str());
		hours = atoi(DateStr.substr(7, 2).c_str());
		minutes = atoi(DateStr.substr(10, 2).c_str());
		seconds = atoi(DateStr.substr(13, 2).c_str());
		year = atoi(DateStr.substr(16, 4).c_str());
	}
	if ((year % 4 == 0) && (month > 60)) // 31 + 29 == 60
		++month;
	size_t count = year % 4 - 1;
	while (year > 1970) {
		if (count % 4 == 0)
			day += 1;
		day += 365;
		--count;
		--year;
	}
	day += month - 1; // dont forget that fabruary has 29 days at leap year
	hours += day * 24;
	minutes += hours * 60;
	seconds += minutes * 60;
	m_time = static_cast<time_t>(seconds);
}

}; // namespace koohar
