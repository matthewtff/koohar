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
    Connect
  };

public:
  HttpParser ();
  virtual ~HttpParser () {}

  /**
   * Makes parser to eat more data and parse it. Could be called multiple
   * times.
   * @param Data Pointer to data that should be parsed.
   * @param Size Size of data pointed ealrier.
   * @return false on parse error, true otherwise.
   */
  bool update (const char* Data, const unsigned int Size);
  bool isBad () const { return m_state == State::OnParseError; }
  bool isComplete () const { return m_state == State::OnComplete; }
  Method method () const { return m_method; }
  std::string uri () const { return m_uri; }
  Version version () const { return m_version; }

  std::string header (const std::string& HeaderName) {
    return m_headers[HeaderName];
  }

  std::string cookie (const std::string& CookieName) {
    return m_cookies[CookieName];
  }

  std::string rawBody () const { return m_body; }

protected:
  Method m_method;
  std::string m_uri;
  Version m_version;
  StringMap m_headers;
  std::string m_body;
  StringMap m_cookies;

private:

  static const size_t MaxTokenSize = 4096;

  enum class State {
    OnMethod,
    OnUri,
    OnHttpVersion,
    OnHeaderName,
    OnHeaderValue,
    OnBody,
    OnComplete,
    OnParseError
  };

  typedef void (HttpParser::*StateCallback) (const char ch);

  void parseMethod (const char ch);
  void parseUri (const char ch);
  void parseHttpVersion (const char ch);
  void parseHeaderName (const char ch);
  void parseHeaderValue (const char ch);
  void parseBody (const char ch);

  void parseCookies (const std::string& CookieStr);

private:
  // Number of callbacks intentionally left blank, to statically assert it
  // against really initialized callbacks and number of states;
  static StateCallback m_callbacks[];
  static std::regex m_cookie_regex;

  State m_state;
  std::string m_token;
  std::string m_current_header;

  std::uint64_t m_content_length;

}; // class HttpParser

} // namespace koohar

#endif // koohar_http_parser_hh
