#ifndef koohar_client_response_hh
#define koohar_client_response_hh

#include "http.hh"
#include "http_parser.hh"

namespace koohar {

class ClientResponse {
 public:
  ClientResponse();
  ClientResponse(ClientResponse&&) = default;

  bool Update(const char* data, const unsigned int size) {
    return http_parser_.Update(data, size);
  }
  bool Valid() const { return http_parser_.IsComplete(); }

  std::string Header(const std::string& header_name) {
    return http_parser_.Header(header_name);
  }

  HTTP::Method method() const { return http_parser_.method(); }
  HTTP::Version version() const { return http_parser_.version(); }
  std::string body() const { return http_parser_.body(); }
  int status_code() const { return http_parser_.status_code(); }

 private:
  HttpParser http_parser_;
};  // class ClientResponse

}  // namespace koohar

#endif  // koohar_client_response_hh
