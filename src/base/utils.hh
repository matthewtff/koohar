#ifndef koohar_utils_hh
#define koohar_utils_hh

#include <map>
#include <ostream>
#include <string>

namespace koohar {

typedef std::map<std::string, std::string> StringMap;

template <typename T, size_t N>
char (&ArraySizeHelper(const T (&array)[N]))[N];

template <typename Array>
constexpr size_t array_size(const Array& array) {
  return sizeof(ArraySizeHelper(array));
}

// Length of string without terminating null character.
template <size_t N>
constexpr size_t string_length(const char (&)[N]) {
  return N - 1;
}

std::ostream& DLOG();

void NOTREACHED();

} // namespace koohar

#endif // koohar_utils_hh
