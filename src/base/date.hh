#ifndef koohar_date_hh
#define koohar_date_hh

#include <ctime>
#include <string>

namespace koohar {

class Date {
 public:
  Date();
  explicit Date(const std::string& date_string);

  std::string ToString() const;
  bool Parse(const std::string& date_string);

 private:
  bool ParseRFC1123(const std::string& date_string);
  bool ParseRFC1036(const std::string& date_string);
  bool ParseANSIC(const std::string& date_string);
  bool ParseDate(const std::string& date_string, const char* date_format);

  std::tm date_;
}; // class Date

} // namespace koohar

#endif // koohar_date_hh
