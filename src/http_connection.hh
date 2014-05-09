#ifndef koohar_http_connection_hh
#define koohar_http_connection_hh

#include <list>
#include <memory>
#include <map>
#include <string>

#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "server_config.hh"
#include "request.hh"

namespace koohar {

class Response;

class HttpConnection : public boost::enable_shared_from_this<HttpConnection>
{
public:
  static const unsigned int MaxRequestSize = 65536; // 64KB

  typedef boost::shared_ptr<HttpConnection> Pointer;

  typedef std::function<void(koohar::Request&, koohar::Response&)> UserFunc;

  typedef std::list<std::vector<char>> DataBuffers;

public:
  static Pointer create(boost::asio::io_service& IoService,
                        UserFunc UserCallFunction,
                        const ServerConfig& Config);

  ~HttpConnection();

  boost::asio::ip::tcp::socket& socket() { return m_socket; }
  void start();
  void write(const char* Data, const std::size_t Size);
  void close() { m_close_socket = true; }
  void setUserFunction(UserFunc UserCallFunction);

public:

private:
  HttpConnection(boost::asio::io_service& IoService,
                 UserFunc UserCallFunction,
                 const ServerConfig& Config);
  void handleRead(const boost::system::error_code& Error,
                  const std::size_t BytesTransferred);
  void handleWrite(const boost::system::error_code& Error,
                   const std::size_t BytesTransferred);
private:
private:
  ServerConfig m_config;
  boost::asio::ip::tcp::socket m_socket;
  std::string m_answer;
  char m_request_buffer[MaxRequestSize];

  Request m_request;
  UserFunc m_user_call_function;
  DataBuffers m_buffers;
  int m_writing_operations;
  bool m_close_socket;
}; // class HttpConnection

} // namespace koohar

#endif // koohar_http_connection_hh
