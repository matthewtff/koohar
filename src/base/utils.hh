#ifndef koohar_utils_hh
#define koohar_utils_hh

#include <map>
#include <ostream>
#include <string>

#ifndef NDEBUG
#include <iostream>
#endif // NDEBUG

namespace koohar {

typedef std::map<std::string, std::string> StringMap;

template <typename T, size_t N>
constexpr size_t array_size(const T (&)[N]) {
  return N;
}

// Length of string without terminating null character.
template <size_t N>
constexpr size_t string_length(const char (&)[N]) {
  static_assert(N > 0, "String is not null terminated!");
  return N - 1;
}

enum LogLevel {
  kError,
  kInfo,
};

std::ostream& LOG(const LogLevel level);
std::ostream& NOTREACHED();

} // namespace koohar

#endif // koohar_utils_hh
