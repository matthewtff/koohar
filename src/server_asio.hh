#ifndef koohar_server_asio_hh
#define koohar_server_asio_hh

#include <boost/asio.hpp>

#include "http_connection.hh"
#include "server_config.hh"

namespace koohar {

class ServerAsio : public ServerConfig {
public:
  ServerAsio(const unsigned short Port = 80);

  virtual ~ServerAsio() = default;

  void listen(HttpConnection::UserFunc UserCallFunction);

  unsigned short port() const { return m_port; }

  void stop() { m_io_service.stop(); }

private:
  bool accept();
  void handleAccept(HttpConnection::Pointer NewConnection,
                    const boost::system::error_code& Error);

private:
  const unsigned short m_port;
  boost::asio::io_service m_io_service;
  boost::asio::ip::tcp::acceptor m_acceptor;

  HttpConnection::UserFunc m_user_call_function;
};  // class ServerAsio

}  // namespace koohar

#endif // koohar_server_asio_hh
