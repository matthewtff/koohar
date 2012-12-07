#include "server_asio.hh"

#include <boost/bind.hpp>

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
			m_user_call_function, dynamic_cast<ServerConfig*>(this));
	
	m_acceptor.async_accept(new_connection->socket(),
		boost::bind(&ServerAsio::handleAccept, this, new_connection,
			boost::asio::placeholders::error
		)
	);
}

void ServerAsio::handleAccept (HttpConnection::Pointer NewConnection,
	const boost::system::error_code& Error)
{
	if (!Error) {
		NewConnection->setUserCallFunction(m_user_call_function);
		NewConnection->start();
	}
	
	accept();
}

} // namespace koohar