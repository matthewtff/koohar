#ifndef koohar_server_config_hh
#define koohar_server_config_hh

#include <string>
#include <forward_list>
#include <map>

namespace koohar {

class Request;

class ServerConfig {
public:

  void setStaticDir (const std::string& Directory) { m_static_dir = Directory; }
  void setStaticUrl (const std::string& Url) { m_static_urls.push_front(Url); }
  void setUseSSL (const bool UseSSL) { m_use_ssl = UseSSL; }

  bool isStaticUrl (const Request& Req) const;
  std::string getStaticDir () const { return m_static_dir; }
  bool getUseSSL () const { return m_use_ssl; }

  void load (const std::string& FileName);

  // Returns empty string if no page for specified code found.
  std::string getErrorPage (const unsigned short Code) const;

private:

  typedef std::forward_list<std::string> StringList;

  typedef std::map<unsigned short, std::string> StringMap;

private:

  std::string m_static_dir;
  StringList m_static_urls;
  StringMap m_error_pages;
  bool m_use_ssl;

}; // class ServerConfig

} // namespace koohar

#endif // koohar_server_config_hh
