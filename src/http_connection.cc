#include "http_connection.hh"

#include <sstream>

#include <boost/bind.hpp>

#include "base/file.hh"
#include "base/utils.hh"
#include "response.hh"
#include "static_transfer.hh"

using boost::asio::ip::tcp;

namespace koohar {

HttpConnection::Pointer HttpConnection::create (
    boost::asio::io_service& IoService,
    UserFunc UserCallFunction,
    const ServerConfig& Config) {
  return Pointer(new HttpConnection(IoService, UserCallFunction, Config));
}

void HttpConnection::start () {
  m_socket.async_read_some (
      boost::asio::buffer(m_request_buffer, MaxRequestSize),
      boost::bind(&HttpConnection::handleRead, shared_from_this(),
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred));
}

void HttpConnection::write (const char* Data, std::size_t Size) {
  if (!Data || !Size)
    return;

  std::vector<char> buffer(Data, Data + Size);

  boost::asio::async_write(
      m_socket,
      boost::asio::buffer(buffer),
      boost::bind(&HttpConnection::handleWrite,
                  shared_from_this(),
                  boost::asio::placeholders::error,
                  boost::asio::placeholders::bytes_transferred));

  m_buffers.emplace_back(buffer);

  ++m_writing_operations;
}

void HttpConnection::setUserFunction (UserFunc UserCallFunction) {
  m_user_call_function = UserCallFunction;
}

HttpConnection::~HttpConnection() {
  DLOG() << "HttpConnection::~HttpConnection()" << std::endl;
}

// private

HttpConnection::HttpConnection(boost::asio::io_service& io_service,
                               UserFunc user_call_function,
                               const ServerConfig& config)
    : m_config(config),
      m_socket(io_service),
      m_user_call_function(user_call_function),
      m_writing_operations(0),
      m_close_socket(false)
{}

void HttpConnection::handleRead (const boost::system::error_code& Error,
                                 const std::size_t BytesTransferred) {
  if (Error) 
    return;

  Response res {shared_from_this()};
  res.header("Server", "koohar.app");

  if (!m_request.update(m_request_buffer, BytesTransferred)) {
    res.writeHead(m_request.errorCode());
    res.end();
    return;
  }

  if (m_config.isStaticUrl(m_request)) {
    StaticTransfer static_transfer {m_request, std::move(res), m_config};
    static_transfer.Serve();
  } else if (m_user_call_function) {
    m_user_call_function(m_request, res);
  } else {
    DLOG() << "HttpConnection[callback_error] : Callback was not set";
  }
}

void HttpConnection::handleWrite (const boost::system::error_code& Error,
                                  const std::size_t BytesTransferred) {
  if (!Error && BytesTransferred == 0)
    DLOG() << "HttpConnection[write_error] : transferred 0 bytes";

  if (m_writing_operations > 0)
    --m_writing_operations;
  if (m_close_socket && m_writing_operations == 0)
    m_socket.close();
}

} // namespace koohar
