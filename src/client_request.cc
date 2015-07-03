#include "client_request.hh"

#include <sstream>

namespace koohar {

ClientRequest::ClientRequest(const std::string& url)
    : ClientRequest(HTTP::Method::Get, url) {}

ClientRequest::ClientRequest(const HTTP::Method method, const std::string& url)
    : method_(method), version_{1, 0} {
  if (!Parse(url)) {
    LOG << "Invalid url requiested: " << url << std::endl;
    NOTREACHED();
  }
}

void ClientRequest::SetHeader(const std::string& header_name,
                              const std::string& header_value) {
  headers_[header_name] = header_value;
}

bool ClientRequest::HasHeader(const std::string& header_name) const {
  return headers_.find(header_name) != headers_.end();
}

std::string ClientRequest::GetHeaderValue(
      const std::string& header_name) const {
  const StringMap::const_iterator header = headers_.find(header_name);
  if (header == headers_.end()) {
    return std::string();
  }
  return header->first;
}

std::string ClientRequest::AsString() const {
  if (!valid()) {
    LOG << "Cannot get HTTP request string for invalid request" << std::endl;
    NOTREACHED();
    return std::string();
  }
  std::ostringstream request_stream;
  request_stream << HTTP::MethodToString(method_) << " " <<
      path() << " " << HTTP::VersionToString(version_) << HTTP::kLineDelimiter;
  for (const auto& header : headers_) {
    request_stream << header.first << HTTP::kHeaderDelimiter
        << header.second << HTTP::kLineDelimiter;
  }
  request_stream << HTTP::kLineDelimiter;
  return request_stream.str();
}

}  // namespace koohar
