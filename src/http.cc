#include "http.hh"

#include "base/utils.hh"

namespace koohar {
namespace HTTP {

extern const char kHeaderDelimiter[3] = ": ";
extern const char kLineDelimiter[3] = "\r\n";

std::string MethodToString(const HTTP::Method method) {
  switch (method) {
    case HTTP::Method::Options : return "OPTIONS";
    case HTTP::Method::Get : return "GET";
    case HTTP::Method::Head : return "HEAD";
    case HTTP::Method::Post : return "POST";
    case HTTP::Method::Put : return "PUT";
    case HTTP::Method::Delete : return "DELETE";
    case HTTP::Method::Trace : return "TRACE";
    case HTTP::Method::Connect : return "CONNECT";
  }
  NOTREACHED() << "We should be able to handle every HTTP method";
  return "GET";
}

std::string VersionToString(const HTTP::Version version) {
  std::string result("HTTP/");
  result = result.append(std::to_string(version.major_));
  result = result.append(".");
  result = result.append(std::to_string(version.minor_));
  return result;
}

}  // namespace HTTP
}  // namesapce koohar
