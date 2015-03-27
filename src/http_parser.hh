#ifndef koohar_http_parser_hh
#define koohar_http_parser_hh

#include <cstdint>
#include <string>
#include <regex>

#include "uri_parser.hh"

namespace koohar {

/**
 * This is FSM based http parser.
 */
class HttpParser : public UriParser {
 public:
  struct Version {
    unsigned short int m_major;
    unsigned short int m_minor;
  };

  enum class Method {
    Options,
    Get,
    Head,
    Post,
    Put,
    Delete,
    Trace,
    Connect,
  };

  HttpParser() = default;
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

  Method method() const { return method_; }
  std::string uri() const { return uri_; }
  Version version() const { return version_; }
  std::string body() const { return body_; }

 protected:
  Method method_;
  std::string uri_;
  Version version_;
  StringMap headers_;
  std::string body_;
  StringMap cookies_;

 private:
  enum class State {
    OnMethod,
    OnUri,
    OnHttpVersion,
    OnHeaderName,
    OnHeaderValue,
    OnBody,
    OnComplete,
    OnParseError,
  };

  void ParseMethod (const char ch);
  void ParseUri (const char ch);
  void ParseHttpVersion (const char ch);
  void ParseHeaderName (const char ch);
  void ParseHeaderValue (const char ch);
  void ParseBody (const char ch);

  void ParseCookies (const std::string& CookieStr);

  State state_ = State::OnMethod;
  std::string token_;
  std::string current_header_;

  std::uint64_t content_length_ = 0;
}; // class HttpParser

} // namespace koohar

#endif // koohar_http_parser_hh
