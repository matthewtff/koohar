#include "server_asio.hh"

#include <functional>

#include "base/utils.hh"

using boost::asio::ip::tcp;

namespace koohar {

ServerAsio::ServerAsio(const unsigned short port)
    : port_(port),
      acceptor_(io_service_, tcp::endpoint(tcp::v4(), port)) {
}

void ServerAsio::Listen(HttpConnection::UserFunc user_call_function) {
  user_call_function_ = user_call_function;
  Accept();
  io_service_.run();
}

// private

bool ServerAsio::Accept() {
  HttpConnection::Pointer new_connection =
      HttpConnection::Create(acceptor_.get_io_service(),
                             user_call_function_,
                             *this);

  try {
    acceptor_.async_accept(
        new_connection->Socket(),
        std::bind(&ServerAsio::HandleAccept,
                  this,
                  new_connection,
                  std::placeholders::_1));
  } catch (boost::exception& e) {
    LOG << "error accepting." << std::endl;
    return false;
  }
  return true;
}

void ServerAsio::HandleAccept(HttpConnection::Pointer new_connection,
                              const boost::system::error_code& error) {
  Accept();
  if (error) {
    return;
  }
  new_connection->set_user_function(user_call_function_);
  new_connection->Start();
}

}  // namespace koohar
