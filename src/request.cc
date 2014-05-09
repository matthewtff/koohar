#include "request.hh"

#ifdef _WIN32

#include <winsock2.h>

#else /* _WIN32 */

#include <cerrno>

#endif /* !_WIN32 */

namespace koohar {

bool Request::contains (const std::string& Url) const {
  return uri().find(Url) != std::string::npos;
}

bool Request::corresponds (const std::string& StaticUrl) const {
  return uri().find(StaticUrl) == 0;
}

} // namespace koohar
