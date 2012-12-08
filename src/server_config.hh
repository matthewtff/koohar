#ifndef koohar_server_config_hh
#define koohar_server_config_hh

#include <string>
#include <list>

namespace koohar {

class Request;

class ServerConfig {

public:

	enum Option {
		SET_STATIC_DIR,
		SET_STATIC_URL
	};

public:

	void config (Option Key, const std::string& Value);

	void setStaticDir (const std::string& Directory);
	void setStaticUrl (const std::string& Url);
	void setUseSSL (bool UseSSL);

	bool isStaticUrl (const Request& Req);
	std::string getStaticDir (void) const;
	bool getUseSSL (void) const;

	void load (const std::string& FileName);

private:

	typedef std::list<std::string> StringList;

private:

	std::string m_static_dir;
	StringList m_static_urls;
	bool m_use_ssl;

}; // class ServerConfig

} // namespace koohar

#endif // koohar_server_config_hh
