#include "server_config.hh"

#include "request.hh"

namespace koohar {

void ServerConfig::config (Option Key, const std::string& Value)
{
	switch (Key) {
		case SET_STATIC_DIR:
			setStaticDir(Value);
		break;
		case SET_STATIC_URL:
			setStaticUrl(Value);
		break;
		default:
		break;
	}
}

void ServerConfig::setStaticDir (const std::string& Directory)
{
	m_static_dir = Directory;
}

void ServerConfig::setStaticUrl (const std::string& Url)
{
	m_static_urls.push_back(Url);
}

bool ServerConfig::isStaticUrl (const Request& Req)
{
	for (auto static_url = m_static_urls.begin();
		static_url != m_static_urls.end(); ++static_url)
	{
		if (Req.corresponds(*static_url))
			return true;
	}

	return false;
}

std::string ServerConfig::getStaticDir (void)
{
	return m_static_dir;
}

} // namespace koohar
