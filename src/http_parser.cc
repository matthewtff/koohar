#include "http_parser.hh"

#include <cctype>
#include <cstdlib>

#include "base/utils.hh"
#include "regex.hh"

namespace koohar {

HTTP::Method operator++(HTTP::Method& method) {
  using Method = HTTP::Method;
  switch (method) {
    case Method::Options: return method = Method::Get;
    case Method::Get: return method = Method::Head;
    case Method::Head: return method = Method::Post;
    case Method::Post: return method = Method::Put;
    case Method::Put: return method = Method::Delete;
    case Method::Delete: return method = Method::Trace;
    case Method::Trace: return method = Method::Connect;
    case Method::Connect: return method = Method::Options;
  }
}

HttpParser::HttpParser(const bool is_client) : is_client_(is_client) {
  state_ = is_client_ ? State::OnHttpVersion : State::OnMethod;
}

bool HttpParser::Update(const char* data, const unsigned int size) {
  using StateCallback = void (HttpParser::*)(const char ch);
  static const StateCallback callbacks[] = {
    &HttpParser::ParseMethod,
    &HttpParser::ParseUri,
    &HttpParser::ParseHttpVersion,
    &HttpParser::ParseStatusCode,
    &HttpParser::ParseReasonPhrase,
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
  static const std::map<std::string, HTTP::Method> methods = {
    { "OPTIONS", HTTP::Method::Options },
    { "GET", HTTP::Method::Get },
    { "HEAD", HTTP::Method::Head },
    { "POST", HTTP::Method::Post },
    { "PUT", HTTP::Method::Put },
    { "DELETE", HTTP::Method::Delete },
    { "TRANCE", HTTP::Method::Trace },
    { "CONNECT", HTTP::Method::Connect },
  };

  if (ch != ' ') {
    token_.append(1, ch);
    return;
  }

  // Finished consuming method.
  const auto method = std::find_if(methods.begin(), methods.end(),
      [&](const std::pair<std::string, HTTP::Method>& pair) {
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
  static const size_t expected_length = string_length("HTTP/x.x");
  const char delimiter = is_client_ ? ' ' : '\n';
  if (ch == delimiter) {
    if (token_.length() != expected_length) {
      state_ = State::OnParseError;
      return;
    }
    state_ = is_client_ ? State::OnStatusCode : State::OnHeaderName;

    switch (token_[5]) {
      case '0':
        version_.major_ = 0;
      break;
      case '1':
        version_.major_ = 1;
      break;
      case '2':
        version_.major_ = 2;
      break;
      default:
        state_ = State::OnParseError;
      break;
    }

    switch (token_[7]) {
      case '0':
        version_.minor_ = 0;
      break;
      case '1':
        version_.minor_ = 1;
      break;
      case '9':
        version_.minor_ = 9;
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

void HttpParser::ParseStatusCode(const char ch) {
  if (ch == ' ') {
    if (token_.length() > 3) {  // All statuses has maximum 3 digits.
      state_ = State::OnParseError;
    } else {
      status_code_ = std::atoi(token_.c_str());
      state_ = status_code_ > 0 ? State::OnReasonPhrase : State::OnParseError;
    }
    token_.erase();
  } else {
    token_.append(1, ch);
  }
}

void HttpParser::ParseReasonPhrase(const char ch) {
  if (ch == '\n') {
    state_ = State::OnHeaderName;
  }
}

void HttpParser::ParseHeaderName(const char ch) {
  if (ch == '\n') {
    state_ =
        (content_length_ || is_client_) ? State::OnBody : State::OnComplete;
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
    } else if (current_header_ == "cookie" && !is_client_) {
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
    if (method_ == HTTP::Method::Post) {
      ParseQuery(body_);
    }
  }
}

void HttpParser::ParseCookies(const std::string& CookieStr) {
  static const re::regex cookie_regex {"([^=]+)=?([^;]*)(;)?[:space]?"};
  static const re::regex_constants::match_flag_type flags =
      re::regex_constants::match_default;

  std::string::const_iterator start = CookieStr.begin();
  const std::string::const_iterator end = CookieStr.end();
  re::match_results<std::string::const_iterator> what;
  while (re::regex_search(start, end, what, cookie_regex, flags)) {
    const std::string cookie_name (what[1].first, what[1].second);
    const std::string cookie_value (what[2].first, what[2].second);
    cookies_[cookie_name] = cookie_value;
    start = what[0].second;
  }
}

} // namespace koohar
