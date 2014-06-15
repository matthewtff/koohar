#include "response.hh"

#include <algorithm>
#include <cerrno>

#include "base/filemapping.hh"
#include "base/utils.hh"

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

Response::Response (HttpConnection::Pointer Connection) :
  m_headers_allowed(true), m_connection(Connection)
{}

void Response::writeHead(const unsigned short State) {
  if (!m_headers_allowed)
    return;
  std::string head {"HTTP/1.1 "};
  head += States[State] + "\r\n";
  transfer(head.c_str(), head.length());
}

void Response::header(const std::string& HeaderName,
                      const std::string& HeaderValue,
                      const bool Replace) {
  if (!m_headers_allowed || HeaderName.empty() || HeaderValue.empty())
    return;
  std::string& hdr = m_headers[HeaderName];
  hdr = (hdr.empty() || Replace)
    ? HeaderValue
    : (hdr.append("; ").append(HeaderValue));
}

/**
 * Actually you should be very carefull to this method cause it
 * does not seem to replace existing cookie value.
 */
bool Response::cookie(const std::string& CookieName,
                      const std::string& CookieValue) {
  const std::string cookie_str = CookieName + "=" + CookieValue + "; Path=/;";
  if (CookieName.empty() || CookieValue.empty())
    return false;
  // Dont replace all cookies, just add
  header("Set-Cookie", cookie_str, false);
  return true;
}

void Response::body(const std::string& String) {
  body(String.c_str(), String.length());
}

void Response::body(const void* Buffer, const off_t BufferSize) {
  sendHeaders();
  transfer(Buffer, BufferSize);
}

bool Response::sendFile(const File::Handle Handle,
                        const off_t Size,
                        const off_t Offset) {
  sendHeaders();

  FileMapping mapping {Handle};
  const char* mapped_file = mapping.map(Size, Offset);

  if (!mapped_file)
    return false;

  transfer(static_cast<const void*>(mapped_file), Size);
  return true;
}

void Response::end(const std::string& String) {
  return end(String.c_str(), String.length());
}

void Response::end(const void* Buffer, const off_t BufferSize) {
  sendHeaders();
  body(Buffer, BufferSize);
  m_connection->close();
}

void Response::end() {
  sendHeaders();
  m_connection->close();
}

void Response::redirect(const std::string& Url) {
  writeHead(302);
  header("Location", Url);
}

void Response::sendJSON(const JSON::Object& Obj) {
  writeHead(200);
  header("Content-Type", "application/json");
  end(Obj.toString());
}

void Response::badRequest() {
  writeHead(400);
  end();
}

// private

void Response::transfer(const void* Buffer, const off_t BufferSize) {
  m_connection->write(static_cast<const char*>(Buffer), BufferSize);
}

void Response::sendHeaders() {
  static const char HeaderDelimiter[] {": "};
  static const char LineDelimiter[] {"\r\n"};

  if (!m_headers_allowed)
    return;
  for (const auto& header : m_headers) {
    transfer(header.first.c_str(), header.first.length());
    transferString(HeaderDelimiter);
    transfer(header.second.c_str(), header.second.length());
    transferString(LineDelimiter);
  }
  transferString(LineDelimiter);
  m_headers.clear();
  m_headers_allowed = false;
}

} // namespace koohar
