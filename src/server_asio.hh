#ifndef koohar_server_asio_hh
#define koohar_server_asio_hh

#include <boost/asio.hpp>

#include "http_connection.hh"
#include "server_config.hh"

namespace koohar {

class ServerAsio : public ServerConfig {
 public:
  ServerAsio(const unsigned short Port = 80);

  void Listen(HttpConnection::UserFunc user_call_function);
  void Stop() { io_service_.stop(); }

  unsigned short port() const { return port_; }

 private:
  bool Accept();
  void HandleAccept(HttpConnection::Pointer new_connection,
                    const boost::system::error_code& error);

  const unsigned short port_;
  boost::asio::io_service io_service_;
  boost::asio::ip::tcp::acceptor acceptor_;

  HttpConnection::UserFunc user_call_function_;
};  // class ServerAsio

}  // namespace koohar

#endif  // koohar_server_asio_hh
