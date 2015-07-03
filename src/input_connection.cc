#include "input_connection.hh"

#include <functional>
#include <sstream>

#include "base/file.hh"
#include "base/utils.hh"
#include "response.hh"
#include "static_transfer.hh"

using boost::asio::ip::tcp;

namespace {
const char kFaviconUrl[] = "/favicon.ico";
}  // anoymous namespace

namespace koohar {

InputConnection::Pointer InputConnection::Create (
    boost::asio::io_service& io_service,
    UserFunc user_call_function,
    const ServerConfig& config) {
  return Pointer(new InputConnection(io_service, user_call_function, config));
}

void InputConnection::Start() {
  socket_.async_read_some (
      boost::asio::buffer(request_buffer_, kMaxRequestSize),
      std::bind(&InputConnection::HandleRead,
                shared_from_this(),
                std::placeholders::_1,
                std::placeholders::_2));
}

void InputConnection::Write(const char* data, std::size_t size) {
  if (!data || !size) {
    return;
  }

  if (close_socket_) {
    LOG << "HttpConnection[write_error]: Writing to socket after close";
  }

  buffers_.emplace_back(data, data + size);

  Write();
}

void InputConnection::Close() {
  close_socket_ = true;
  if (buffers_.empty()) {
    socket_.close();
  }
}

// private

InputConnection::InputConnection(boost::asio::io_service& io_service,
                               UserFunc user_call_function,
                               const ServerConfig& config)
    : config_(config),
      socket_(io_service),
      user_call_function_(user_call_function),
      currently_writing_(false),
      close_socket_(false) {
}

void InputConnection::Write() {
  if (currently_writing_) {
    return;
  }
  boost::asio::async_write(
    socket_,
    boost::asio::buffer(buffers_.front()),
    std::bind(&InputConnection::HandleWrite,
              shared_from_this(),
              std::placeholders::_1,
              std::placeholders::_2));
  currently_writing_ = true;
}

void InputConnection::HandleRead(const boost::system::error_code& error,
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

  if (config_.IsStaticUrl(request_) || request_.path() == kFaviconUrl) {
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

void InputConnection::HandleWrite(const boost::system::error_code& error,
                                 const std::size_t bytes_transferred) {
  if (!error && bytes_transferred == 0) {
    LOG << "HttpConnection[write_error] : transferred 0 bytes";
  }

  currently_writing_ = false;
  buffers_.pop_front();

  if (!buffers_.empty()) {
    Write();
  }

  if (close_socket_ && buffers_.empty()) {
    socket_.close();
  }
}

} // namespace koohar
