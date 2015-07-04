#include "output_connection.hh"

#include "client_cache.hh"

namespace asio = boost::asio;

using std::placeholders::_1;
using std::placeholders::_2;

namespace koohar {

void OutputConnection::Run(asio::io_service& io_service,
                           ClientRequest&& request,
                           Callback callback,
                           asio::ip::tcp::resolver::iterator hosts,
                           ClientCache* client_cache) {
  Pointer connection(new OutputConnection(io_service,
                                          std::move(request),
                                          callback,
                                          client_cache));
  connection->Start(hosts);
}

void OutputConnection::Start(asio::ip::tcp::resolver::iterator hosts) {
  if (!request_.valid()) {
    Respond();
  } else {
    asio::async_connect(socket_, hosts,
                        std::bind(&OutputConnection::HandleConnect,
                                  shared_from_this(), _1, _2));
  }
}

OutputConnection::OutputConnection(asio::io_service& io_service,
                                   ClientRequest&& request,
                                   Callback callback,
                                   ClientCache* client_cache)
    : socket_(io_service),
      bytes_sent_(0u),
      request_(std::move(request)),
      callback_(callback),
      client_cache_(client_cache) {
}

void OutputConnection::Respond() {
  if (callback_) {
    const bool should_use_cache =
        !response_.Valid() || response_.status_code() == 304;
    if (cached_response_.Valid() && should_use_cache) {
      response_ = std::move(cached_response_);
    } else if (response_.Valid() && response_.status_code() == 200) {
      client_cache_->SetCacheEntry(request_, response_);
    }
    callback_(std::move(request_), std::move(response_));
  }
  callback_ = nullptr;
}

void OutputConnection::Write() {
  auto buffer = asio::buffer(request_text_.c_str() + bytes_sent_,
                             request_text_.size() - bytes_sent_);
  asio::async_write(socket_, buffer,
                    std::bind(&OutputConnection::HandleWrite,
                              shared_from_this(), _1, _2));
}

void OutputConnection::Read() {
  asio::async_read(socket_,
                   asio::buffer(response_buffer_, kMaxResponseSize),
                   std::bind(&OutputConnection::HandleRead,
                             shared_from_this(), _1, _2));
}

void OutputConnection::HandleConnect(const boost::system::error_code& error,
                                     asio::ip::tcp::resolver::iterator) {
  if (error) {
    return Respond();
  }
  cached_response_ = client_cache_->PrepareRequest(request_);
  request_text_ = request_.AsString();
  Write();
  Read();
}

void OutputConnection::HandleRead(const boost::system::error_code& error,
                                  const std::size_t bytes_transferred) {
  if (!response_.Update(response_buffer_, bytes_transferred) || error) {
    if (error != asio::error::eof) {
      LOG(kInfo) << "Error reading from output connection: " <<
          error.message() << std::endl;
    }
    response_.Finish();
    return Respond();
  }
  Read();
}

void OutputConnection::HandleWrite(const boost::system::error_code& error,
                                   const std::size_t bytes_transferred) {
  if (error) {
    return Respond();
  }
  bytes_sent_ += bytes_transferred;
  if (bytes_sent_ != request_text_.size()) {
    Write();
  }
}

}  // namespace koohar
