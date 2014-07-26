#ifndef koohar_request_hh
#define koohar_request_hh

#include <string>

#include "http_parser.hh"

namespace koohar {

class Request : public HttpParser {
 public:
  static int errorCode () { return 400; }

  Request() = default;
  Request(Request&&) = default;

  Request& operator=(Request&&) = default;

  bool contains (const std::string& Url) const;
  bool corresponds (const std::string& StaticUrl) const;
}; // class Request

} // namespace koohar

#endif // koohar_request_hh