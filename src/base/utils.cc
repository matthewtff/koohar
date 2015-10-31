#include "base/utils.hh"

#include <cassert>
#include <ostream>

namespace koohar {

std::ostream& LOG(const LogLevel level) {
  switch (level) {
    case LogLevel::kError: return std::cerr;
    case LogLevel::kInfo: return std::cout;
  }
  return std::cout;
}

std::ostream& NOTREACHED() {
  assert(false);
  return LOG(LogLevel::kError);
}

} // namespace koohar
