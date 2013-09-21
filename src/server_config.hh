#ifndef koohar_server_config_hh
#define koohar_server_config_hh

#include <string>
#include <list>
#include <map>

namespace koohar {

class Request;

class ServerConfig {

public:

  enum Option {
    SET_STATIC_DIR,
    SET_STATIC_URL
  };

public:

  void config (const Option Key, const std::string& Value);

  inline void setStaticDir (const std::string& Directory);
  void setStaticUrl (const std::string& Url);
  void setUseSSL (const bool UseSSL);

  bool isStaticUrl (const Request& Req);
  std::string getStaticDir () const;
  bool getUseSSL () const;

  void load (const std::string& FileName);

  std::string getErrorPage (const unsigned short Code) const;

private:

  typedef std::list<std::string> StringList;

  typedef std::map<unsigned short, std::string> StringMap;

private:

  std::string m_static_dir;
  StringList m_static_urls;
  StringMap m_error_pages;
  bool m_use_ssl;

}; // class ServerConfig

} // namespace koohar

#endif // koohar_server_config_hh
