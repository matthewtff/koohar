#include "base/utils.hh"

#include <cassert>
#include <ostream>

namespace koohar {

std::ostream& LOG(const LogLevel level) {
  switch (level) {
    case kError: return std::cerr;
    case kInfo: return std::cout;
  }
}

std::ostream& NOTREACHED() {
  assert(false);
  return LOG(kError);
}

} // namespace koohar
