#ifndef koohar_http_parser_hh
#define koohar_http_parser_hh

#include <cstdint>
#include <string>

#include "http.hh"
#include "uri_parser.hh"

namespace koohar {

/**
 * This is FSM based http parser.
 */
class HttpParser : public UriParser {
 public:
  HttpParser(const bool is_client);
  HttpParser(HttpParser&&) = default;

  HttpParser& operator=(HttpParser&&) = default;

  /**
   * Makes parser to eat more data and parse it. Could be called multiple
   * times.
   * @param Data Pointer to data that should be parsed.
   * @param Size Size of data pointed ealrier.
   * @return false on parse error, true otherwise.
   */
  bool Update(const char* data, const unsigned int size);
  bool IsBad() const { return state_ == State::OnParseError; }
  bool IsComplete() const { return state_ == State::OnComplete; }

  std::string Header(const std::string& header_name) {
    return headers_[header_name];
  }

  std::string Cookie(const std::string& cookie_name) {
    return cookies_[cookie_name];
  }

  HTTP::Method method() const { return method_; }
  std::string uri() const { return uri_; }
  HTTP::Version version() const { return version_; }
  std::string body() const { return body_; }
  int status_code() const { return status_code_; }

 protected:
  HTTP::Method method_;
  std::string uri_;
  HTTP::Version version_;
  int status_code_ = 0;
  StringMap headers_;
  std::string body_;
  StringMap cookies_;

 private:
  enum class State {
    OnMethod,
    OnUri,
    OnHttpVersion,
    OnStatusCode,
    OnReasonPhrase,
    OnHeaderName,
    OnHeaderValue,
    OnBody,
    OnComplete,
    OnParseError,
  };

  void ParseMethod (const char ch);
  void ParseUri (const char ch);
  void ParseHttpVersion (const char ch);
  void ParseStatusCode (const char ch);
  void ParseReasonPhrase (const char ch);
  void ParseHeaderName (const char ch);
  void ParseHeaderValue (const char ch);
  void ParseBody (const char ch);

  void ParseCookies (const std::string& CookieStr);

  bool is_client_;
  State state_;
  std::string token_;
  std::string current_header_;

  std::uint64_t content_length_ = 0;
}; // class HttpParser

} // namespace koohar

#endif // koohar_http_parser_hh
