#include "server_config.hh"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
//#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>

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

void ServerConfig::load (const std::string& FileName)
{
	using boost::property_tree::ptree;
	ptree pt;

	//read_json(FileName, pt);
	read_xml(FileName, pt);

	setStaticDir(pt.get<std::string>("config.public_dir"));

	BOOST_FOREACH(ptree::value_type &v, pt.get_child("config.public_urls")) {
		setStaticUrl(v.second.data());
	}
}

void ServerConfig::save (const std::string& FileName)
{
	using boost::property_tree::ptree;
	ptree pt;

	pt.put("config.public_dir", m_static_dir);

	BOOST_FOREACH(const std::string& url, m_static_urls) {
		pt.add("config.public_urls", url);
	}

	//write_json(FileName, pt);
	write_xml(FileName, pt);
}

} // namespace koohar
