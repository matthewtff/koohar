#include "http_parser.hh"

#include <cctype>
#include <cstdlib>

#include "base/utils.hh"

namespace koohar {

HttpParser::Method operator++(HttpParser::Method& method) {
  using Method = HttpParser::Method;
  switch (method) {
    case Method::Options: return method = Method::Get;
    case Method::Get: return method = Method::Head;
    case Method::Head: return method = Method::Post;
    case Method::Post: return method = Method::Put;
    case Method::Put: return method = Method::Delete;
    case Method::Delete: return method = Method::Trace;
    case Method::Trace: return method = Method::Connect;
    case Method::Connect: return method = Method::Options;
    default: NOTREACHED(); return method;
  }
}

bool HttpParser::Update (const char* data, const unsigned int size) {
  using StateCallback = void (HttpParser::*)(const char ch);
  static const StateCallback callbacks[] = {
    &HttpParser::ParseMethod,
    &HttpParser::ParseUri,
    &HttpParser::ParseHttpVersion,
    &HttpParser::ParseHeaderName,
    &HttpParser::ParseHeaderValue,
    &HttpParser::ParseBody,
  };

  static_assert(
      array_size(callbacks) == static_cast<size_t>(State::OnComplete),
      "Number of callbacks should equal to '<number of states> - 2'");

  static const std::size_t kMaxTokenSize = 4096;

  for (unsigned int counter = 0; counter < size; ++counter) {
    if (state_ == State::OnParseError) {
      return false;
    }
    if (token_.length() > kMaxTokenSize || !data[counter]) {
      state_ = State::OnParseError;
      return false; // Prevent buffer overflow.
    }
    if (state_ == State::OnComplete) {
      return true;
    }
    (this->*callbacks[static_cast<size_t>(state_)])(data[counter]);
  }
  return state_ != State::OnParseError;
}

// private

void HttpParser::ParseMethod(const char ch) {
  static const std::map<std::string, Method> methods = {
    { "OPTIONS", Method::Options },
    { "GET", Method::Get },
    { "HEAD", Method::Head },
    { "POST", Method::Post },
    { "PUT", Method::Put },
    { "DELETE", Method::Delete },
    { "TRANCE", Method::Trace },
    { "CONNECT", Method::Connect },
  };

  if (ch != ' ') {
    token_.append(1, ch);
    return;
  }

  // Finished consuming method.
  const auto method = std::find_if(methods.begin(), methods.end(),
      [&](const std::pair<std::string, Method>& pair) {
        return token_ == pair.first;
      });
  const bool correct_method = method != methods.end();
  if (correct_method) {
    method_ = method->second;
  }

  token_.erase();
  state_ = correct_method ? State::OnUri : State::OnParseError;
}

void HttpParser::ParseUri(const char ch) {
  if (ch == ' ') {
    uri_ = token_;
    state_ = State::OnHttpVersion;
    token_.erase();
    if (!Parse(uri_)) {  // Bad uri.
      state_ = State::OnParseError;
    }
  } else {
    token_.append(1, ch);
  }
}

void HttpParser::ParseHttpVersion(const char ch) {
  if (ch == '\n') {
    if (token_.length() != string_length("HTTP/x.x")) {
      state_ = State::OnParseError;
      return;
    }
    state_ = State::OnHeaderName;

    switch (token_[5]) {
      case '0':
        version_.m_major = 0;
      break;
      case '1':
        version_.m_major = 1;
      break;
      case '2':
        version_.m_major = 2;
      break;
      default:
        state_ = State::OnParseError;
      break;
    }

    switch (token_[7]) {
      case '0':
        version_.m_minor = 0;
      break;
      case '1':
        version_.m_minor = 1;
      break;
      case '9':
        version_.m_minor = 9;
      break;
      default:
        state_ = State::OnParseError;
      break;
    }
    token_.erase();
  } else if (ch != '\r') {
    token_.append(1, ch);
  }
}

void HttpParser::ParseHeaderName(const char ch) {
  if (ch == '\n') {
    state_ = content_length_ ? State::OnBody : State::OnComplete;
    token_.erase();
  } else if (ch == ' ') {
    current_header_ = token_;
    state_ = State::OnHeaderValue;
    token_.erase();
  } else if (ch != ':') {
    token_.append(1, tolower(ch));
  }
}

void HttpParser::ParseHeaderValue(const char ch) {
  if (ch == '\n') {
    headers_[current_header_] = token_;
    if (current_header_ == "content-length") {
      content_length_ = std::atol(token_.c_str());
    } else if (current_header_ == "cookie") {
      ParseCookies(token_);
    }
    state_ = State::OnHeaderName;
    token_.erase();
  } else if (ch != '\r') {
    token_.append(1, ch);
  }
}

void HttpParser::ParseBody(const char ch) {
  body_.append(1, ch);
  token_.append(1, ch);
  if (token_.length() == content_length_) {
    state_ = State::OnComplete;
    if (method_ == Method::Post) {
      ParseQuery(body_);
    }
  }
}

void HttpParser::ParseCookies(const std::string& CookieStr) {
  static const std::regex cookie_regex {"([^=]+)=?([^;]*)(;)?[:space]?"};
  static const std::regex_constants::match_flag_type flags =
    std::regex_constants::match_default;

  std::string::const_iterator start = CookieStr.begin();
  const std::string::const_iterator end = CookieStr.end();
  std::match_results<std::string::const_iterator> what;
  while (std::regex_search(start, end, what, cookie_regex, flags)) {
    const std::string cookie_name (what[1].first, what[1].second);
    const std::string cookie_value (what[2].first, what[2].second);
    cookies_[cookie_name] = cookie_value;
    start = what[0].second;
  }
}

} // namespace koohar
