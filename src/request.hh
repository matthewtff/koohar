#ifndef koohar_request_hh
#define koohar_request_hh

#include <string>

#include "http_parser.hh"

namespace koohar {

class Request : public HttpParser {
 public:
  static int error_code() { return 400; }

  Request() = default;
  Request(Request&&) = default;

  Request& operator=(Request&&) = default;

  bool Contains(const std::string& url) const;
  bool Corresponds(const std::string& static_url) const;
}; // class Request

} // namespace koohar

#endif // koohar_request_hh
