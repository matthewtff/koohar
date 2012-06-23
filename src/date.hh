#ifndef koohar_date_hh
#define koohar_date_hh

#include <string>
#include <ctime>

#include <iostream>

namespace koohar {

// According to rfc2616 date in HTTP could be represented by any of these standarts:
// 1) RFC1123 : 'Tue, 09 Sep 2011 23:34:12 GMT', fixed length : 29,
// 2) RFC850  : 'Wednesday, 21-Sep-11 18:46:37 GMT', dynamic length : 29-33,
// 3) ASCTIME : 'Sat May 20 15:21:51 2000', fixed length : 24.
// http://www.w3.org/Protocols/rfc2616/rfc2616.html - HTTP/1.1
// As usual we want to be able to parse string to time stamp and back.
// TODO:Also opportunity to add years, months, days, hours, minutes and seconds is a nice idea.

enum DateFormat {
	RFC1123,
	RFC850,
	ASCTIME
}; // DateFormat

class Date {
public:
	Date (size_t Region = 4) { m_time = ::time(NULL); incHours(Region); } // MSK region by default, for now...
	Date (const time_t TimeStamp) : m_time(TimeStamp) {}
	Date (const std::string& DateStr);
	std::string toString (DateFormat Format = RFC1123) const;
	void time(const time_t NewTime) { m_time = NewTime; }
	time_t time() const { return m_time; }
	int monthToNumber (const std::string& MonthStr) const;
	std::string numberToMonth (const int MonthNumber) const; 
	int monthToDay (const std::string& MonthStr) const;
	int dayToMonth (const int DayNumber, int& Day, bool leap_year = false) const;
	Date& incDays (const size_t DayNumber);
	Date& incHours (const size_t HourNumber);
	Date& incMinutes (const size_t MinuteNumber);
private:
	time_t m_time;
	static const char months[12][4];
	static const unsigned short day_per_month[];
	static const char day_of_week[7][10];

	void parse(const std::string& DateStr, DateFormat Format);
}; // class Date

}; // namespace koohar

#endif // koohar_date_hh
