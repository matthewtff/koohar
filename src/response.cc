#include "response.hh"

#include <algorithm>
#include <cerrno>

#include "base/filemapping.hh"
#include "base/json.hh"
#include "base/utils.hh"
#include "http.hh"

namespace {
const char kContentTypeHeader[] = "Content-Type";
const char kLocationHeader[] = "Location";
const char kSetCookieHeader[] = "Set-Cookie";

const char kJSONMimeType[] = "application/json";
}  // anonymous namespace

namespace koohar {

Response::StateMap Response::States = {
  {100, "100 Continue"},
  {101, "101 Switching Protocols"},
  {102, "102 Processing"},

  {200, "200 OK"},
  {201, "201 Created"},
  {202, "202 Accepted"},
  {203, "203 Non-Authoritative Information"},
  {204, "204 No Content"},
  {205, "205 Reset Content"},
  {206, "206 Partial Content"},
  {207, "207 Multi-Status"},
  {226, "226 IM Used"},

  {300, "300 Multiple Choises"},
  {301, "301 Moved Permamently"},
  {302, "302 Found"},
  {303, "303 See Other"},
  {304, "304 Not Modified"},
  {305, "305 Use Proxy"},
  {307, "307 Temporary Redirect"},

  {400, "400 Bad Request"},
  {401, "401 Unauthorized"},
  {402, "402 Payment Required"},
  {403, "403 Forbidden"},
  {404, "404 Not Found"},
  {405, "405 Method Not Allowed"},
  {406, "406 Not Acceptable"},
  {407, "407 Proxy Authentication Required"},
  {408, "408 Request Timeout"},
  {409, "409 Conflict"},
  {410, "410 Gone"},
  {411, "411 Length Required"},
  {412, "412 Precondition Failed"},
  {413, "413 Request Entity Too Large"},
  {414, "414 Request-URI Too Long"},
  {415, "415 Unsupported Media Type"},
  {416, "416 Requested Range Not Satisfiable"},
  {417, "417 Expectation Failed"},
  {418, "418 I'm a teapot"}, // First april joke : rfc 2424
  {422, "422 Unprocessable Entity"},
  {423, "423 Loked"},
  {424, "424 Failed Dependency"},
  {425, "425 Unordered Collection"},
  {426, "426 Upgrade Required"},
  {449, "449 Retry Width"},
  {456, "456 Unrecoverable Error"},

  {500, "500 Internal Server Error"},
  {501, "501 Not Implemented"},
  {502, "502 Bad Gateway"},
  {503, "503 Service Unavailable"},
  {504, "504 Gateway Timeout"},
  {505, "505 HTTP Version No Supported"},
  {506, "506 Variant Also Negotiates"},
  {507, "507 Insufficient Storage"},
  {509, "509 Bandwidth Limit Exceeded"},
  {510, "510 Not Extended"},
};

Response::Response(InputConnection::Pointer Connection)
    : headers_allowed_(true), connection_(Connection) {
}

void Response::WriteHead(const unsigned short state) {
  if (!headers_allowed_)
    return;
  std::string head{"HTTP/1.1 "};
  head += States[state] + HTTP::kLineDelimiter;
  Transfer(head.c_str(), head.length());
}

void Response::Header(const std::string& header_name,
                      const std::string& header_value,
                      const bool replace) {
  if (!headers_allowed_ || header_name.empty() || header_value.empty()) {
    return;
  }
  std::string& hdr = headers_[header_name];
  hdr = (hdr.empty() || replace) ?
      header_value : (hdr.append("; ").append(header_value));
}

/**
 * Actually you should be very carefull to this method cause it
 * does not seem to replace existing cookie value.
 */
bool Response::Cookie(const std::string& cookie_name,
                      const std::string& cookie_value) {
  const std::string cookie_str = cookie_name + "=" + cookie_value + "; Path=/;";
  if (cookie_name.empty() || cookie_value.empty())
    return false;
  // Dont replace all cookies, just add one.
  Header(kSetCookieHeader, cookie_str, false);
  return true;
}

void Response::Body(const std::string& data) {
  Body(data.c_str(), data.length());
}

void Response::Body(const void* buffer, const off_t buffer_size) {
  SendHeaders();
  Transfer(buffer, buffer_size);
}

bool Response::SendFile(const File::Handle file_handle,
                        const off_t size,
                        const off_t offset) {
  SendHeaders();

  FileMapping mapping{file_handle};
  const char* mapped_file = mapping.Map(size, offset);

  if (!mapped_file) {
    return false;
  }

  Transfer(static_cast<const void*>(mapped_file), size);
  return true;
}

void Response::End(const std::string& data) {
  return End(data.c_str(), data.length());
}

void Response::End(const void* buffer, const off_t buffer_size) {
  SendHeaders();
  Body(buffer, buffer_size);
  connection_->Close();
}

void Response::End() {
  SendHeaders();
  connection_->Close();
}

void Response::Redirect(const std::string& url) {
  WriteHead(302);
  Header(kLocationHeader, url);
}

void Response::SendJSON(const JSON::Object& object) {
  WriteHead(200);
  Header(kContentTypeHeader, kJSONMimeType);
  End(object.ToString());
}

void Response::BadRequest() {
  WriteHead(400);
  End();
}

// private

void Response::Transfer(const void* buffer, const off_t buffer_size) {
  connection_->Write(static_cast<const char*>(buffer), buffer_size);
}

void Response::SendHeaders() {
  if (!headers_allowed_)
    return;
  for (const auto& header : headers_) {
    Transfer(header.first.c_str(), header.first.length());
    TransferString(HTTP::kHeaderDelimiter);
    Transfer(header.second.c_str(), header.second.length());
    TransferString(HTTP::kLineDelimiter);
  }
  TransferString(HTTP::kLineDelimiter);
  headers_.clear();
  headers_allowed_ = false;
}

}  // namespace koohar
