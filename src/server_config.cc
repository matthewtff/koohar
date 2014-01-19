#include "server_config.hh"

#include <algorithm>
#include <functional>
#include <fstream>
#include <cstdlib>

#include "base/json.hh"
#include "base/utils.hh"
#include "request.hh"

namespace koohar {

bool ServerConfig::isStaticUrl (const Request& Req) const {
  return m_static_urls.cend() !=
  std::find_if(m_static_urls.cbegin(),
               m_static_urls.cend(),
               std::bind(&Request::corresponds,
                         &Req,
                         std::placeholders::_1));
}

void ServerConfig::load (const std::string& FileName) {
  std::ifstream f(FileName.c_str());
  const std::string contents((std::istreambuf_iterator<char>(f)),
                             std::istreambuf_iterator<char>());

  JSON::Object config;
  if (contents.size() != config.parse(contents)) {
    DLOG() << "Error parsing json info.";
    return;
  }

  if (config["public_dir"].type() == JSON::Type::String)
    setStaticDir(config["public_dir"].getString());

  if (config["use_ssl"].type() == JSON::Type::Boolean)
    setUseSSL(config["use_ssl"].getBoolean());

  if (config["public_urls"].type() == JSON::Type::Array) {
    const auto& urls = config["public_urls"].getArray();
    for (const JSON::Object& url : urls) {
      if (url.type() == JSON::Type::String)
        setStaticUrl(url.getString());
    }
  }

  if (config["error_pages"].type() == JSON::Type::Array) {
    auto& pages = config["error_pages"].getArray();
    for (JSON::Object& page : pages) {
      if (page.type() != JSON::Type::Collection)
        continue;

      if (page["code"].type() == JSON::Type::Integer &&
          page["path"].type() == JSON::Type::String) {
        const long code = page["code"].getInteger();
        m_error_pages[code] = page["path"].getString();
      }
    }
  }
}

std::string ServerConfig::getErrorPage (const unsigned short Code) const {
  const StringMap::const_iterator page_iterator = m_error_pages.find(Code);
  if (page_iterator != m_error_pages.end())
    return page_iterator->second;
  return std::string();
}

} // namespace koohar
