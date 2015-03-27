#include "server_config.hh"

#include <algorithm>
#include <functional>
#include <fstream>
#include <cstdlib>

#include "base/json.hh"
#include "base/utils.hh"
#include "request.hh"

namespace {
const char kCode[] = "code";
const char kErrorPages[] = "error_pages";
const char kPath[] = "path";
const char kPublicDir[] = "public_dir";
const char kPublicURLs[] = "public_urls";
const char kUseSSL[] = "use_ssl";
}  // anonymous namespace

namespace koohar {

bool ServerConfig::IsStaticUrl(const Request& req) const {
  return static_urls_.cend() != std::find_if(
      static_urls_.cbegin(),
      static_urls_.cend(),
      std::bind(&Request::Corresponds, &req, std::placeholders::_1));
}

void ServerConfig::Load(const std::string& file_name) {
  std::ifstream input(file_name.c_str());
  if (!input) {
    return;
  }
  const std::string contents((std::istreambuf_iterator<char>(input)),
                             std::istreambuf_iterator<char>());

  JSON::Object config = JSON::Parse(contents);

  if (config[kPublicDir].type() == JSON::Type::String) {
    set_static_dir(config[kPublicDir].GetString());
  }

  if (config[kUseSSL].type() == JSON::Type::Boolean) {
    set_use_ssl(config[kUseSSL].GetBoolean());
  }

  if (config[kPublicURLs].type() == JSON::Type::Array) {
    const auto& urls = config[kPublicURLs].GetArray();
    for (const JSON::Object& url : urls) {
      if (url.type() == JSON::Type::String) {
        AddStaticUrl(url.GetString());
      }
    }
  }

  if (config[kErrorPages].type() == JSON::Type::Array) {
    auto& pages = config[kErrorPages].GetArray();
    for (JSON::Object& page : pages) {
      if (page.type() != JSON::Type::Collection) {
        continue;
      }

      if (page[kCode].type() == JSON::Type::Integer &&
          page[kPath].type() == JSON::Type::String) {
        const long code = page[kCode].GetInteger();
        error_pages_[code] = page[kPath].GetString();
      }
    }
  }
}

std::string ServerConfig::GetErrorPage(const unsigned short code) const {
  const ErrorPagesMap::const_iterator page_iterator = error_pages_.find(code);
  if (page_iterator != error_pages_.cend()) {
    return page_iterator->second;
  }
  return std::string();
}

}  // namespace koohar
