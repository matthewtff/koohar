#include "server_asio.hh"

#include <boost/bind.hpp>

#include <iostream>

#include "utils.hh"

using boost::asio::ip::tcp;

namespace koohar {

ServerAsio::ServerAsio (unsigned short Port) : m_port(Port),
  m_acceptor(m_io_service, tcp::endpoint(tcp::v4(), Port))
{
  accept();
}

void ServerAsio::listen (HttpConnection::UserFunc UserCallFunction)
{
  m_user_call_function = UserCallFunction;
  m_io_service.run();
}

// private

void ServerAsio::accept ()
{
  HttpConnection::Pointer new_connection =
      HttpConnection::create(m_acceptor.get_io_service(),
                             m_user_call_function,
                             *this);

  try {
    m_acceptor.async_accept(new_connection->socket(),
      boost::bind(&ServerAsio::handleAccept, this, new_connection,
        boost::asio::placeholders::error
      )
    );
  } catch (boost::exception& e) {
    DLOG() << "Error accepting..." << std::endl;
  }
}

void ServerAsio::handleAccept (HttpConnection::Pointer NewConnection,
  const boost::system::error_code& Error)
{
  if (!Error) {
    NewConnection->setUserFunction(m_user_call_function);
    NewConnection->start();
  }
  
  accept();
}

} // namespace koohar
