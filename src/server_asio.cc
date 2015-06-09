#include "server_asio.hh"

#include <functional>
#include <tuple>

#include "base/utils.hh"

using boost::asio::ip::tcp;

namespace koohar {
namespace {

std::size_t GenerateTimerId() {
  static std::size_t timer_id = 0u;
  return ++timer_id;
}

}  // anonymous namespace

ServerAsio::ServerAsio(const unsigned short port)
    : port_(port),
      acceptor_(io_service_, tcp::endpoint(tcp::v4(), port)) {
}

void ServerAsio::Listen(HttpConnection::UserFunc user_call_function) {
  user_call_function_ = user_call_function;
  Accept();
  io_service_.run();
}

ServerAsio::TimeoutHandle ServerAsio::SetTimeout(
    std::chrono::milliseconds timeout,
    TimerCallback callback) {
  return SetTimer(timeout, callback, &ServerAsio::HandleTimeout);
}

ServerAsio::TimeoutHandle ServerAsio::SetInterval(
    std::chrono::milliseconds timeout,
    TimerCallback callback) {
  return SetTimer(timeout, callback, &ServerAsio::HandleInterval);
}

void ServerAsio::ClearTimeout(TimeoutHandle timeout_handle) {
  TimersMap::iterator timer = user_timers_.find(timeout_handle);
  if (timer != user_timers_.end()) {
    timer->second.cancel();
  }
  // Timer would be erase in |HandleTimeout|.
}

void ServerAsio::ClearInterval(TimeoutHandle interval_handle) {
  TimersMap::iterator timer = user_timers_.find(interval_handle);
  if (timer != user_timers_.end()) {
    timer->second.cancel();
  }
  user_timers_.erase(timer);
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

ServerAsio::TimeoutHandle ServerAsio::SetTimer(
    std::chrono::milliseconds timeout,
    TimerCallback callback,
    HandleTimer handle_timer) {
  const TimeoutHandle timer_handle = GenerateTimerId();
  std::pair<TimersMap::iterator, bool> timer(user_timers_.end(), false);
  // Iff we're reusing handles, then skip existing ones.
  while (!timer.second) {
    timer = user_timers_.emplace(std::piecewise_construct,
                                 std::forward_as_tuple(timer_handle),
                                 std::forward_as_tuple(io_service_, timeout));
  }
  timer.first->second.async_wait(std::bind(handle_timer,
                                           this,
                                           timeout,
                                           timer_handle,
                                           callback,
                                           std::placeholders::_1));
  return timer_handle;
}

void ServerAsio::HandleTimeout(std::chrono::milliseconds /* timeout */,
                               TimeoutHandle timeout_handle,
                               TimerCallback callback,
                               const boost::system::error_code& error) {
  if (error != boost::asio::error::operation_aborted) {
    callback();
  }
  user_timers_.erase(timeout_handle);
}

void ServerAsio::HandleInterval(std::chrono::milliseconds timeout,
                                TimeoutHandle interval_handle,
                                TimerCallback callback,
                                const boost::system::error_code& error) {
  if (error == boost::asio::error::operation_aborted) {
    return;
  }
  callback();
  TimersMap::iterator timer = user_timers_.find(interval_handle);
  if (timer != user_timers_.end()) {
    timer->second.expires_at(std::chrono::steady_clock::now() + timeout);
    timer->second.async_wait(std::bind(&ServerAsio::HandleInterval,
                                       this,
                                       timeout,
                                       interval_handle,
                                       callback,
                                       std::placeholders::_1));
  }
}

}  // namespace koohar
