#include "uri_parser.hh"

#include <cctype>
#include <regex>

namespace {

char FromHex(const char ch) {
  return std::isdigit(ch) ? ch - '0' : std::tolower(ch) - 'a' + 10;
}

std::string Decode(const std::string& Uri) {
  std::string result;
  for (unsigned int counter = 0; counter < Uri.length(); ++counter) {
    if (Uri[counter] == '%') {
      if (Uri.length() - counter < 2) {
        return result;
      }
      result.append(1,
                    FromHex(Uri[counter + 1]) << 4 | FromHex(Uri[counter + 2]));
      counter += 2;
    } else {
      result += Uri[counter] == '+' ? ' ' : Uri[counter];
    }
  }
  return result;
}

}  // anonymous namespace

namespace koohar {

bool UriParser::Parse(const std::string& uri) {
  static const std::regex uri_regex
      ("^(([^:/?#]+):)?(//([^/?#]*))?([^?#]*)(\\?([^#]*))?(#(.*))?");

  const std::string decoded_uri = Decode(uri);
  const std::regex_constants::match_flag_type flags =
      std::regex_constants::match_default;

  const std::string::const_iterator start = decoded_uri.begin();
  const std::string::const_iterator end = decoded_uri.end();
  std::match_results<std::string::const_iterator> what;
  if (!std::regex_search(start, end, what, uri_regex, flags)) {
    return false;
  }

  scheme_ = std::string(what[2].first, what[2].second);
  authority_ = std::string(what[4].first, what[4].second);
  path_ = std::string(what[5].first, what[5].second);
  query_ = std::string(what[7].first, what[7].second);
  fragment_ = std::string(what[9].first, what[9].second);

  ParseQuery(query_);
  return true;
}

std::string UriParser::Body(const std::string& query_name) const {
  StringMap::const_iterator data_iterator = queries_.find(query_name);
  if (data_iterator == queries_.cend()) {
    return std::string();
  }
  return data_iterator->second;
}

// protected

void UriParser::ParseQuery(const std::string& query_string) {
  static const std::regex query_regex ("([^&=]+)=?([^&]*)(&)?");
  const std::regex_constants::match_flag_type flags =
      std::regex_constants::match_default;
  std::string::const_iterator start = query_string.begin();
  const std::string::const_iterator end = query_string.end();
  std::match_results<std::string::const_iterator> what;
  while (std::regex_search(start, end, what, query_regex, flags)) {
    std::string query_name (what[1].first, what[1].second);
    std::string query_value (what[2].first, what[2].second);
    queries_[query_name] = query_value;
    start = what[0].second;
  }
}

}  // namespace koohar
