#ifndef koohar_http_connection_hh
#define koohar_http_connection_hh

#include <list>
#include <memory>
#include <map>
#include <string>

#include <boost/asio.hpp>

#include "server_config.hh"
#include "request.hh"

namespace koohar {

class Response;

class HttpConnection : public std::enable_shared_from_this<HttpConnection> {
 public:
  static const unsigned int kMaxRequestSize = 65536;  // 64KB

  typedef std::shared_ptr<HttpConnection> Pointer;
  typedef std::function<void(koohar::Request&&, koohar::Response&&)> UserFunc;
  typedef std::list<std::vector<char>> DataBuffers;

  static Pointer Create(boost::asio::io_service& io_service,
                        UserFunc user_call_function,
                        const ServerConfig& config);

  ~HttpConnection();

  boost::asio::ip::tcp::socket& Socket() { return socket_; }
  void Start();
  void Write(const char* data, const std::size_t size);
  void Close();
  void set_user_function(UserFunc user_call_function) {
    user_call_function_ = user_call_function;
  }

  bool closed() const { return close_socket_; }

 private:
  HttpConnection(boost::asio::io_service& io_service,
                 UserFunc user_call_function,
                 const ServerConfig& config);
  void HandleRead(const boost::system::error_code& error,
                  const std::size_t bytes_transferred);
  void HandleWrite(const boost::system::error_code& error,
                   const std::size_t bytes_transferred);

  ServerConfig config_;
  boost::asio::ip::tcp::socket socket_;
  char request_buffer_[kMaxRequestSize];
  Request request_;
  UserFunc user_call_function_;
  DataBuffers buffers_;
  int writing_operations_;
  bool close_socket_;

  std::string url_;
}; // class HttpConnection

} // namespace koohar

#endif // koohar_http_connection_hh
