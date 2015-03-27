#include "base/utils.hh"

#ifdef NDEBUG
#include <cassert>
#include <iostream>
#include <utility>
#else // NDEBUG
#include <ostream>
#endif // NDEBUG

namespace koohar {

void NOTREACHED() {
#ifdef NDEBUG
  std::assert(false);
#endif // NDEBUG
}

} // namespace koohar
