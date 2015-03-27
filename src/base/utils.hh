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
char (&ArraySizeHelper(const T (&array)[N]))[N];

#define array_size(array) (sizeof(koohar::ArraySizeHelper(array)))


// Length of string without terminating null character.
template <size_t N>
size_t string_length(const char (&)[N]) {
  static_assert(N > 0, "String is not null terminated!");
  return N - 1;
}

#ifndef NDEBUG

#define LOG (std::cout)

#else // NDEBUG

class DummyOstream : public std::ostream {};
#define LOG dummy;

#endif // NDEBUG

void NOTREACHED();

} // namespace koohar

#endif // koohar_utils_hh
