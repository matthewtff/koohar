#ifndef koohar_server_config_hh
#define koohar_server_config_hh

#include <string>
#include <forward_list>
#include <map>

namespace koohar {

class Request;

class ServerConfig {
 public:
  bool IsStaticUrl(const Request& req) const;
  void AddStaticUrl(const std::string& url) { static_urls_.push_front(url); }

  void Load(const std::string& file_name);

  // Get "error page" url.
  // Returns empty string if no page for specified code found.
  std::string GetErrorPage(const unsigned short code) const;

  void set_static_dir(const std::string& directory) { static_dir_ = directory; }
  std::string static_dir() const { return static_dir_; }

  bool use_ssl() const { return use_ssl_; }
  void set_use_ssl(const bool use_ssl) { use_ssl_ = use_ssl; }

 private:
  typedef std::forward_list<std::string> StringList;
  typedef std::map<unsigned short, std::string> ErrorPagesMap;

  std::string static_dir_;
  StringList static_urls_;
  ErrorPagesMap error_pages_;
  bool use_ssl_;
};  // class ServerConfig

}  // namespace koohar

#endif // koohar_server_config_hh
