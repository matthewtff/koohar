#include "uri_parser.hh"

#include <cctype>

namespace koohar {

const std::regex UriParser::m_uri_regex
  ("^(([^:/?#]+):)?(//([^/?#]*))?([^?#]*)(\\?([^#]*))?(#(.*))?");

const std::regex UriParser::m_query_regex ("([^&=]+)=?([^&]*)(&)?");

bool UriParser::parse (const std::string& uri) {
  const std::string decoded_uri = decode (uri);
  const std::regex_constants::match_flag_type flags =
      std::regex_constants::match_default;

  std::string::const_iterator start = decoded_uri.begin();
  std::string::const_iterator end = decoded_uri.end();
  std::match_results<std::string::const_iterator> what;
  if (!std::regex_search(start, end, what, m_uri_regex, flags))
    return false;

  m_scheme = std::string(what[2].first, what[2].second);
  m_authority = std::string(what[4].first, what[4].second);
  m_path = std::string(what[5].first, what[5].second);
  m_query = std::string(what[7].first, what[7].second);
  m_fragment = std::string(what[9].first, what[9].second);

  parseQuery(m_query);

  return true;
}

std::string UriParser::body (const std::string& QueryName) {
  return m_queries[QueryName];
}

// protected

void UriParser::parseQuery (const std::string& QueryString) {
  const std::regex_constants::match_flag_type flags =
      std::regex_constants::match_default;
  std::string::const_iterator start = QueryString.begin();
  std::string::const_iterator end = QueryString.end();
  std::match_results<std::string::const_iterator> what;
  while (std::regex_search(start, end, what, m_query_regex, flags)) {
    std::string query_name (what[1].first, what[1].second);
    std::string query_value (what[2].first, what[2].second);
    m_queries[query_name] = query_value;
    start = what[0].second;
  }
}

// private

char UriParser::fromHex (const char ch) {
  return isdigit(ch)
    ? ch - '0'
    : tolower(ch) - 'a' + 10;
}

std::string UriParser::decode (const std::string& Uri) {
  std::string ret_str;
  for (unsigned int counter = 0; counter < Uri.length(); ++counter) {
    if (Uri[counter] == '%') {
      if (Uri.length() - counter < 2)
        return ret_str;
      ret_str.append(1,
        fromHex(Uri[counter + 1]) << 4 | fromHex(Uri[counter + 2]));
      counter += 2;
    } else {
      ret_str += Uri[counter] == '+' ? ' ' : Uri[counter];
    }
  }
  return ret_str;
}

} // namespace koohar
