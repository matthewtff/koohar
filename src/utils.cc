#include "utils.hh"

#ifndef _DEBUG

#include <iostream>

#else // _DEBUG

#include <ostream>

#endif // _DEBUG

class DummyOstream : public std::ostream {};

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

} // namespace koohar
