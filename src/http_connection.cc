#include "http_connection.hh"

#include <functional>
#include <sstream>

#include "base/file.hh"
#include "base/utils.hh"
#include "response.hh"
#include "static_transfer.hh"

using boost::asio::ip::tcp;

namespace koohar {

HttpConnection::Pointer HttpConnection::Create (
    boost::asio::io_service& io_service,
    UserFunc user_call_function,
    const ServerConfig& config) {
  return Pointer(new HttpConnection(io_service, user_call_function, config));
}

void HttpConnection::Start() {
  socket_.async_read_some (
      boost::asio::buffer(request_buffer_, kMaxRequestSize),
      std::bind(&HttpConnection::HandleRead,
                shared_from_this(),
                std::placeholders::_1,
                std::placeholders::_2));
}

void HttpConnection::Write(const char* data, std::size_t size) {
  if (!data || !size) {
    return;
  }

  if (close_socket_) {
    LOG << "HttpConnection[write_error]: Writing to socket after close";
  }

  buffers_.emplace_back(data, data + size);

  boost::asio::async_write(
      socket_,
      boost::asio::buffer(*(buffers_.rbegin())),
      std::bind(&HttpConnection::HandleWrite,
                shared_from_this(),
                std::placeholders::_1,
                std::placeholders::_2));

  ++writing_operations_;
}

void HttpConnection::Close() {
  close_socket_ = true;
  if (writing_operations_ == 0) {
    socket_.close();
  }
}

HttpConnection::~HttpConnection() {
}

// private

HttpConnection::HttpConnection(boost::asio::io_service& io_service,
                               UserFunc user_call_function,
                               const ServerConfig& config)
    : config_(config),
      socket_(io_service),
      user_call_function_(user_call_function),
      writing_operations_(0),
      close_socket_(false) {
}

void HttpConnection::HandleRead(const boost::system::error_code& error,
                                const std::size_t bytes_transferred) {
  if (error) {
    return;
  }

  Response res{shared_from_this()};
  res.Header("Server", "koohar");

  if (!request_.Update(request_buffer_, bytes_transferred)) {
    res.WriteHead(request_.error_code());
    res.End();
    return;
  } else if (!request_.IsComplete()) {
    return Start();
  }

  url_ = request_.uri();

  if (config_.IsStaticUrl(request_)) {
    StaticTransfer static_transfer{std::move(request_),
                                   std::move(res),
                                   config_};
    static_transfer.Serve();
  } else if (user_call_function_) {
    user_call_function_(std::move(request_), std::move(res));
  } else {
    // TODO(matthewtff): Implement default callback to close connections.
    LOG << "HttpConnection[callback_error] : Callback was not set";
  }
}

void HttpConnection::HandleWrite(const boost::system::error_code& error,
                                 const std::size_t bytes_transferred) {
  if (!error && bytes_transferred == 0) {
    LOG << "HttpConnection[write_error] : transferred 0 bytes";
  }

  if (writing_operations_ > 0) {
    --writing_operations_;
  }
  if (close_socket_ && writing_operations_ == 0) {
    socket_.close();
  }
}

} // namespace koohar
