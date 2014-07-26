#include "http_parser.hh"

#include <cctype>
#include <cstdlib>

#include "base/utils.hh"

namespace koohar {

HttpParser::Method operator++ (HttpParser::Method& Meth) {
  using Method = HttpParser::Method;
  switch (Meth) {
    case Method::Options: return Meth = Method::Get;
    case Method::Get: return Meth = Method::Head;
    case Method::Head: return Meth = Method::Post;
    case Method::Post: return Meth = Method::Put;
    case Method::Put: return Meth = Method::Delete;
    case Method::Delete: return Meth = Method::Trace;
    case Method::Trace: return Meth = Method::Connect;
    case Method::Connect: return Meth = Method::Options;
    default: NOTREACHED();
  }
}

bool HttpParser::update (const char* Data, const unsigned int Size) {
  typedef void (HttpParser::*StateCallback) (const char ch);
  static const StateCallback callbacks[] = {
    &HttpParser::parseMethod,
    &HttpParser::parseUri,
    &HttpParser::parseHttpVersion,
    &HttpParser::parseHeaderName,
    &HttpParser::parseHeaderValue,
    &HttpParser::parseBody
  };

  static_assert(
      array_size(callbacks) == static_cast<size_t>(State::OnComplete),
      "Number of callbacks should equal to '<number of states> - 2'");

  static const std::size_t MaxTokenSize = 4096;

  for (unsigned int counter = 0; counter < Size; ++counter) {
    if (m_state == State::OnParseError)
      return false;
    if (m_token.length() > MaxTokenSize || !Data[counter]) {
      m_state = State::OnParseError;
      return false; // Prevent buffer overflow.
    }
    if (m_state == State::OnComplete)
      return true;
    (this->*callbacks[static_cast<size_t>(m_state)]) (Data[counter]);
  }
  return m_state != State::OnParseError;
}

// private

void HttpParser::parseMethod (const char ch) {
  static const std::map<std::string, Method> methods = {
    { "OPTIONS", Method::Options },
    { "GET", Method::Get },
    { "HEAD", Method::Head },
    { "POST", Method::Post },
    { "PUT", Method::Put },
    { "DELETE", Method::Delete },
    { "TRANCE", Method::Trace },
    { "CONNECT", Method::Connect }
  };

  if (ch != ' ') {
    m_token.append(1, ch);
    return;
  }

  // Finished consuming method.
  const auto method = std::find_if(methods.begin(), methods.end(),
      [&](const std::pair<std::string, Method>& pair) {
        return m_token == pair.first;
      });
  const bool correct_method = method != methods.end();
  if (correct_method)
    m_method = (*method).second;

  m_token.erase();
  m_state = correct_method ? State::OnUri : State::OnParseError;
}

void HttpParser::parseUri (const char ch) {
  if (ch == ' ') {
    m_uri = m_token;
    m_state = State::OnHttpVersion;
    m_token.erase();
    if (!parse(m_uri)) // Bad uri.
      m_state = State::OnParseError;
  } else {
    m_token.append(1, ch);
  }
}

void HttpParser::parseHttpVersion (const char ch) {
  if (ch == '\n') {
    if (m_token.length() != string_length("HTTP/x.x")) {
      m_state = State::OnParseError;
      return;
    }
    m_state = State::OnHeaderName;

    switch (m_token[5]) {
      case '0':
        m_version.m_major = 0;
      break;
      case '1':
        m_version.m_major = 1;
      break;
      case '2':
        m_version.m_major = 2;
      break;
      default:
        m_state = State::OnParseError;
      break;
    }

    switch (m_token[7]) {
      case '0':
        m_version.m_minor = 0;
      break;
      case '1':
        m_version.m_minor = 1;
      break;
      case '9':
        m_version.m_minor = 9;
      break;
      default:
        m_state = State::OnParseError;
      break;
    }
    m_token.erase();
  } else if (ch != '\r') {
    m_token.append(1, ch);
  }
}

void HttpParser::parseHeaderName (const char ch) {
  if (ch == '\n') {
    m_state = m_content_length ? State::OnBody : State::OnComplete;
    m_token.erase();
  } else if (ch == ' ') {
    m_current_header = m_token;
    m_state = State::OnHeaderValue;
    m_token.erase();
  } else if (ch != ':') {
    m_token.append(1, tolower(ch));
  }
}

void HttpParser::parseHeaderValue (const char ch) {
  if (ch == '\n') {
    m_headers[m_current_header] = m_token;
    if (m_current_header == "content-length") {
      m_content_length = std::atol(m_token.c_str());
    } else if (m_current_header == "cookie") {
      parseCookies(m_token);
    }
    m_state = State::OnHeaderName;
    m_token.erase();
  } else if (ch != '\r') {
    m_token.append(1, ch);
  }
}

void HttpParser::parseBody (const char ch) {
  m_body.append(1, ch);
  m_token.append(1, ch);
  if (m_token.length() == m_content_length) {
    m_state = State::OnComplete;
    if (m_method == Method::Post) {
      parseQuery(m_body);
    }
  }
}

void HttpParser::parseCookies (const std::string& CookieStr) {
  static const std::regex cookie_regex {"([^=]+)=?([^;]*)(;)?[:space]?"};
  static const std::regex_constants::match_flag_type flags =
    std::regex_constants::match_default;

  std::string::const_iterator start = CookieStr.begin();
  const std::string::const_iterator end = CookieStr.end();
  std::match_results<std::string::const_iterator> what;
  while (std::regex_search(start, end, what, cookie_regex, flags)) {
    const std::string cookie_name (what[1].first, what[1].second);
    const std::string cookie_value (what[2].first, what[2].second);
    m_cookies[cookie_name] = cookie_value;
    start = what[0].second;
  }
}

} // namespace koohar
