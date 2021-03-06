#ifndef koohar_uri_parser_hh
#define koohar_uri_parser_hh

#include <string>
#include <map>

#include "base/utils.hh"

namespace koohar {

class UriParser {
 public:
  UriParser() {}
  UriParser(const UriParser&) = default;
  UriParser(UriParser&&) = default;
  virtual ~UriParser() = default;

  UriParser& operator=(UriParser&&) = default;

  bool valid() const { return valid_; }

  bool Parse(const std::string& uri);
  std::string Body(const std::string& query_name) const;

  std::string scheme() const { return scheme_; }
  std::string authority() const { return authority_; }
  std::string path() const { return path_; }
  std::string query() const { return query_; }
  std::string fragment() const { return fragment_; }
  std::string url() const { return url_; }

 protected:
  void ParseQuery(const std::string& query_string);

  StringMap queries_;

  bool valid_ = false;
  std::string scheme_;
  std::string authority_;
  std::string path_;
  std::string query_;
  std::string fragment_;
  std::string url_;
};  // class UriParser

}  // namespace koohar

#endif // koohar_uri_parser_hh
