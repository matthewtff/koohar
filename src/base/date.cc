#include "base/date.hh"

#include <exception>
#include <iomanip>
#include <sstream>

#include "base/utils.hh"

namespace koohar {

Date::Date() {
  const std::time_t local_time = std::time(nullptr);
  const std::tm* local_tm = std::gmtime(&local_time);
  if (local_tm) {
    date_ = *local_tm;
  }
}

Date::Date(const std::string& date_string) {
  Parse(date_string);
}

std::string Date::ToString() const {
  std::ostringstream ss;
  ss << std::put_time(&date_, "%a, %d %b %Y %T GMT");
  return ss.str();
}

bool Date::Parse(const std::string& date_string) {
  return ParseRFC1123(date_string) ||
         ParseRFC1036(date_string) ||
         ParseANSIC(date_string);
}

// private:

bool Date::ParseRFC1123(const std::string& date_string) {
  std::istringstream ss(date_string);
  ss >> std::get_time(&date_, "%a, %d %b %Y %T GMT");
  return !ss.fail();
}

bool Date::ParseRFC1036(const std::string& date_string) {
  static const char* kDaysOfWeek[] = {
    "Monday,",
    "Tuesday,",
    "Wednesday,",
    "Thursday,",
    "Friday,",
    "Saturday,",
    "Sunday,"
  };
  static const char** begin_of_week = kDaysOfWeek;
  static const char** end_of_week = kDaysOfWeek + array_size(kDaysOfWeek);
  if (date_string.find(' ') == std::string::npos) {
    return false;
  }
  std::string day_of_week;
  std::istringstream ss(date_string);
  ss >> day_of_week;

  if (end_of_week == std::find(begin_of_week, end_of_week, day_of_week)) {
    return false;
  }

  ss >> std::get_time(&date_, "%d-%b-%y %T GMT");
  return !ss.fail();
}

bool Date::ParseANSIC(const std::string& date_string) {
  std::istringstream ss(date_string);
  ss >> std::get_time(&date_, "%a %b %d %T %Y");
  return !ss.fail();
}

} // namespace koohar