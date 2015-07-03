#ifndef koohar_input_connection_hh
#define koohar_input_connection_hh

#include <list>
#include <memory>
#include <map>
#include <string>

#include <boost/asio.hpp>

#include "server_config.hh"
#include "request.hh"

namespace koohar {

class Response;

class InputConnection : public std::enable_shared_from_this<InputConnection> {
 public:
  static const unsigned int kMaxRequestSize = 65536;  // 64KB

  typedef std::shared_ptr<InputConnection> Pointer;
  typedef std::function<void(Request&&, Response&&)> UserFunc;
  typedef std::list<std::vector<char>> DataBuffers;

  static Pointer Create(boost::asio::io_service& io_service,
                        UserFunc user_call_function,
                        const ServerConfig& config);

  boost::asio::ip::tcp::socket& Socket() { return socket_; }
  void Start();
  void Write(const char* data, const std::size_t size);
  void Close();

  bool closed() const { return close_socket_; }

 private:
  InputConnection(boost::asio::io_service& io_service,
                 UserFunc user_call_function,
                 const ServerConfig& config);
  void Write();
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
  bool currently_writing_;
  bool close_socket_;

  std::string url_;
}; // class InputConnection

} // namespace koohar

#endif // koohar_input_connection_hh
