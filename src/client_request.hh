#ifndef koohar_client_request_hh
#define koohar_client_request_hh

#include <string>

#include "base/utils.hh"
#include "http.hh"
#include "uri_parser.hh"

namespace koohar {

class ClientRequest : public UriParser {
 public:
  ClientRequest(const std::string& url);
  ClientRequest(const HTTP::Method method, const std::string& url);

  HTTP::Version version() const { return version_; }
  void set_version(const HTTP::Version& version) {
    version_ = version;
  }

  StringMap& headers() { return headers_; }
  const StringMap& headers() const { return headers_; }

  void SetHeader(const std::string& header_name,
                 const std::string& header_value);
  bool HasHeader(const std::string& header_name) const;
  std::string GetHeaderValue(const std::string& header_name) const;

  std::string AsString() const;

 private:
  HTTP::Method method_;
  HTTP::Version version_;

  StringMap headers_;
};  // class ClientRequest


}  // namespace koohar

#endif  // koohar_client_request_hh
