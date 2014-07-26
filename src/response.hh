#ifndef koohar_response_hh
#define koohar_response_hh

#include <map>
#include <string>

#include "base/file.hh"
#include "base/json.hh"
#include "base/utils.hh"
#include "http_connection.hh"

namespace koohar {

class Sender;

class Response {
 public:
  typedef std::map<unsigned short, std::string> StateMap;

  /**
   * States are very often used, so no need to create(allocate) that much
   * memory every time response object created. Thats why static map
   * is created.
   */
  static StateMap States;

  explicit Response(HttpConnection::Pointer Connection);
  Response(Response&&) = default;

  Response& operator=(Response&&) = default;

  /**
   * Sets and sends appropriate http status to client.
   */
  void writeHead(const unsigned short State);

  /**
   * Sets header. Can be called multiple times. If Replace set to false, the
   * value whould be added using semicolon (or how do call that ';' ?).
   */
  void header(const std::string& HeaderName,
              const std::string& HeaderValue,
              const bool Replace = true);

  /**
   * Calls header() method using 'Set-Cookie' HeaderName.
   * Be carefull on using.
   */
  bool cookie(const std::string& CookieName,
              const std::string& CookieValue);

  void body (const std::string& String);

  /**
   * Send some data. Can be called multiple times.
   */
  void body(const void* Buffer, const off_t BufferSize);

  /**
   * Cannot be used whith body() method. After using you still should call
   * the end() method.
   */
  bool sendFile(File::Handle FileHandle,
                const off_t Size,
                const off_t Offset);

  void end (const std::string& String);

  /**
   * Terminates connection to client.
   */
  void end(const void* Buffer, const off_t BufferSize);

  void end();

  /**
   * Sends redirecting header and 302 (Redirect) status. You stll can send
   * some data using body() method and even set additional headers. Do not
   * forget to close connection using end().
   */
  void redirect (const std::string& Url);

  void sendJSON (const JSON::Object& Obj);

  void badRequest ();

private:
  /**
   * body() and end() methods have some identical code, so it moved to
   * private method.
   */
  void transfer(const void* Buffer, const off_t BufferSize);

  template <size_t N>
  void transferString(const char (&Str) [N]) {
    transfer(reinterpret_cast<const void*>(Str),
             static_cast<off_t>(string_length(Str)));
  }

  /**
   * This method handle '\n\r' sending after headers part and setting
   * appropriate value to m_headers_allowed variable.
   */
  void sendHeaders();

private:
  StringMap m_headers;
  bool m_headers_allowed; // Set to true, till any part of body is sent.
  
  HttpConnection::Pointer m_connection;
}; // class Response

}; // namespace koohar

#endif // koohar_response_hh
