#include "base/date.hh"

#include <algorithm>
#include <ctime>
#include <exception>
#include <forward_list>
#include <iomanip>
#include <sstream>

#include "base/utils.hh"

namespace koohar {

Date::Date() {
  const std::time_t local_time = std::time(nullptr);
  gmtime_r(&local_time, &date_);
}

Date::Date(const std::string& date_string) {
  Parse(date_string);
}

std::string Date::ToString() const {
  static const size_t kBufferSize = 512u;
  char buffer[kBufferSize];
  const size_t size =
      std::strftime(buffer, kBufferSize, "%a, %d %b %Y %T GMT", &date_);
  return std::string(buffer, size);
}

bool Date::Parse(const std::string& date_string) {
  return ParseRFC1123(date_string) ||
         ParseRFC1036(date_string) ||
         ParseANSIC(date_string);
}

// private:

bool Date::ParseRFC1123(const std::string& date_string) {
  return ParseDate(date_string, "%a, %d %b %Y %T GMT");
}

bool Date::ParseRFC1036(const std::string& date_string) {
  static std::forward_list<std::string> kDaysOfWeek = {
    "Monday,",
    "Tuesday,",
    "Wednesday,",
    "Thursday,",
    "Friday,",
    "Saturday,",
    "Sunday,",
  };
  if (date_string.find(' ') == std::string::npos) {
    return false;
  }
  std::string day_of_week;
  std::istringstream ss(date_string);
  ss >> day_of_week;

  if (kDaysOfWeek.end() == std::find(kDaysOfWeek.begin(),
                                     kDaysOfWeek.end(),
                                     day_of_week)) {
    return false;
  }

  return ParseDate(ss.str(), "%d-%b-%y %T GMT");
}

bool Date::ParseANSIC(const std::string& date_string) {
  return ParseDate(date_string, "%a %b %d %T %Y");
}

bool Date::ParseDate(const std::string& date_string, const char* date_format) {
#ifdef _WIN32
  std::istringstream ss(date_string);
  ss >> std::get_time(&date_, date_format);
  return !ss.fail();
#else  // _WIN32
  return strptime(date_string.c_str(), date_format, &date_);
#endif  // _WIN32
}

}  // namespace koohar
