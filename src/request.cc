#include "request.hh"

#ifdef _WIN32
#include <winsock2.h>
#else /* _WIN32 */
#include <cerrno>
#endif /* !_WIN32 */

namespace koohar {

Request::Request() : HttpParser(false) {}

bool Request::Contains(const std::string& url) const {
  return uri().find(url) != std::string::npos;
}

bool Request::Corresponds(const std::string& static_url) const {
  return uri().find(static_url) == 0;
}

} // namespace koohar
