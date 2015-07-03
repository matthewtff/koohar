#ifndef koohar_response_hh
#define koohar_response_hh

#include <map>
#include <string>

#include "base/file.hh"
#include "base/utils.hh"
#include "input_connection.hh"

namespace koohar {

namespace JSON {
class Object;
};

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

  explicit Response(InputConnection::Pointer Connection);
  Response(Response&&) = default;

  Response& operator=(Response&&) = default;

  /**
   * Sets and sends appropriate http status to client.
   */
  void WriteHead(const unsigned short State);

  /**
   * Sets header. Can be called multiple times. If Replace set to false, the
   * value whould be added using semicolon (or how do call that ';' ?).
   */
  void Header(const std::string& header_name,
              const std::string& header_value,
              const bool replace);

  void Header(const std::string& header_name,
              const std::string& header_value) {
    Header(header_name, header_value, true);
  }

  /**
   * Calls Header() method using 'Set-Cookie' as header name.
   * Be carefull on using.
   */
  bool Cookie(const std::string& cookie_name,
              const std::string& cookie_value);

  /**
   * Send some data. Can be called multiple times.
   */
  void Body(const void* buffer, const off_t buffer_size);
  void Body(const std::string& data);

  /**
   * Cannot be used whith Body() method. After using you still should call
   * the End() method.
   */
  bool SendFile(File::Handle file_handle,
                const off_t Size,
                const off_t Offset);

  /**
   * Terminates connection to client.
   */
  void End(const void* buffer, const off_t buffer_size);
  void End(const std::string& data);
  void End();

  /**
   * Sends redirecting header and 302 (Redirect) status. You stll can send
   * some data using Body() method and even set additional headers. Do not
   * forget to close connection using End().
   */
  void Redirect(const std::string& url);

  void SendJSON(const JSON::Object& object);

  void BadRequest();

  bool IsComplete() const { return connection_->closed(); }

 private:
  /**
   * Body() and End() methods have some identical code, so it moved to
   * private method.
   */
  void Transfer(const void* buffer, const off_t buffer_size);

  template <size_t N>
  void TransferString(const char (&Str) [N]) {
    Transfer(reinterpret_cast<const void*>(Str),
             static_cast<off_t>(string_length(Str)));
  }

  /**
   * This method handle '\n\r' sending after headers part and setting
   * appropriate value to |headers_allowed_| variable.
   */
  void SendHeaders();

  StringMap headers_;
  bool headers_allowed_;  // Set to true, till any part of body is sent.
  
  InputConnection::Pointer connection_;
};  // class Response

}  // namespace koohar

#endif // koohar_response_hh
