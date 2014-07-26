#include "server_asio.hh"

#include <functional>

#include "base/utils.hh"

using boost::asio::ip::tcp;

namespace koohar {

ServerAsio::ServerAsio(const unsigned short Port)
    : m_port(Port),
      m_acceptor(m_io_service, tcp::endpoint(tcp::v4(), Port)) {
}

void ServerAsio::listen(HttpConnection::UserFunc UserCallFunction) {
  m_user_call_function = UserCallFunction;
  accept();
  m_io_service.run();
}

// private

bool ServerAsio::accept() {
  HttpConnection::Pointer new_connection =
      HttpConnection::create(m_acceptor.get_io_service(),
                             m_user_call_function,
                             *this);

  try {
    m_acceptor.async_accept(
        new_connection->socket(),
        std::bind(&ServerAsio::handleAccept,
                  this,
                  new_connection,
                  std::placeholders::_1));
  } catch (boost::exception& e) {
    LOG << "Error accepting..." << std::endl;
    return false;
  }
  return true;
}

void ServerAsio::handleAccept(HttpConnection::Pointer NewConnection,
                              const boost::system::error_code& Error) {
  accept();
  if (Error)
    return;
  NewConnection->setUserFunction(m_user_call_function);
  NewConnection->start();
}

} // namespace koohar
