#include "server_config.hh"

#include <functional>
#include <cstdlib>
#include <exception>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>

#include "request.hh"

#include <iostream>

namespace koohar {

void ServerConfig::config (const Option Key, const std::string& Value)
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

void ServerConfig::setUseSSL (const bool UseSSL)
{
  m_use_ssl = UseSSL;
}

bool ServerConfig::isStaticUrl (const Request& Req)
{
  return m_static_urls.end() !=
  std::find_if(m_static_urls.begin(),
               m_static_urls.end(),
               std::bind(&Request::corresponds,
                         &Req,
                         std::placeholders::_1));
}

std::string ServerConfig::getStaticDir () const
{
  return m_static_dir;
}

bool ServerConfig::getUseSSL () const
{
  return m_use_ssl;
}

void ServerConfig::load (const std::string& FileName)
{
  //TODO(matthewtff): Change config to JSON.
  using boost::property_tree::ptree;
  ptree pt;

  try {
    read_xml(FileName, pt);
  } catch (std::exception& e) {
    std::cout << "Error reading xml: " << e.what() << std::endl;
    return;
  }

  try {
    setStaticDir(pt.get<std::string>("config.public_dir"));

    BOOST_FOREACH(ptree::value_type &v,
                  pt.get_child("config.public_urls")) {
      setStaticUrl(v.second.data());
    }

    BOOST_FOREACH(ptree::value_type &v,
                  pt.get_child("config.error_pages")) {
      m_error_pages[std::atoi(v.first.data())] =  v.second.data();
    }

    setUseSSL (static_cast<bool>(pt.get<short>("config.use_ssl")));
  } catch (std::exception& e) {
    std::cout << "Error loading config file: " << e.what() << std::endl;
  }
}

std::string ServerConfig::getErrorPage (const unsigned short Code) const
{
  StringMap::const_iterator page_iterator = m_error_pages.find(Code);
  if (page_iterator != m_error_pages.end())
    return page_iterator->second;
  else
    return std::string();
}

} // namespace koohar
