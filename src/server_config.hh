#ifndef koohar_server_config_hh
#define koohar_server_config_hh

#include <string>
#include <forward_list>
#include <map>

namespace koohar {

class Request;

class ServerConfig {
public:
  void setStaticDir(const std::string& directory) { m_static_dir = directory; }
  void setStaticUrl(const std::string& url) { m_static_urls.push_front(url); }
  void setUseSSL(const bool use_ssl) { m_use_ssl = use_ssl; }

  bool isStaticUrl(const Request& req) const;
  std::string getStaticDir() const { return m_static_dir; }
  bool getUseSSL() const { return m_use_ssl; }

  void load(const std::string& file_name);

  // Returns empty string if no page for specified code found.
  std::string getErrorPage(const unsigned short code) const;

private:
  typedef std::forward_list<std::string> StringList;
  typedef std::map<unsigned short, std::string> ErrorPagesMap;

  std::string m_static_dir;
  StringList m_static_urls;
  ErrorPagesMap m_error_pages;
  bool m_use_ssl;
};  // class ServerConfig

}  // namespace koohar

#endif // koohar_server_config_hh
