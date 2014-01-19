#include "base/utils.hh"

#ifdef NDEBUG

#include <iostream>
#include <cassert>

#else // NDEBUG

#include <ostream>

#endif // NDEBUG

#ifndef NDEBUG

class DummyOstream : public std::ostream {};

#endif // !NDEBUG

namespace koohar {

std::ostream& DLOG()
{
#ifdef _DEBUG

  return std::cout;

#else // _DEBUG

  static DummyOstream dummy;
  return dummy;

#endif // _DEBUG
}

void NOTREACHED() {
#ifdef NDEBUG

  std::assert(false);

#endif
}

} // namespace koohar
