#ifndef koohar_server_asio_hh
#define koohar_server_asio_hh

#include <chrono>
#include <functional>
#include <unordered_map>

#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>

#include "http_connection.hh"
#include "server_config.hh"

namespace koohar {

class ServerAsio : public ServerConfig {
 public:
  using TimerCallback = std::function<void()>;
  using TimeoutHandle = std::size_t;
  ServerAsio(const unsigned short Port = 80);

  void Listen(HttpConnection::UserFunc user_call_function);
  void Stop() { io_service_.stop(); }

  unsigned short port() const { return port_; }

  TimeoutHandle SetTimeout(std::chrono::milliseconds timeout,
                           TimerCallback callback);
  TimeoutHandle SetInterval(std::chrono::milliseconds timeout,
                            TimerCallback callback);
  void ClearTimeout(TimeoutHandle timeout_handle);
  void ClearInterval(TimeoutHandle interval_handle);

 private:
  using TimersMap =
      std::unordered_map<TimeoutHandle, boost::asio::steady_timer>;
  using HandleTimer =
      void (ServerAsio::*)(std::chrono::milliseconds,
                           TimeoutHandle,
                           TimerCallback,
                           const boost::system::error_code&);
  bool Accept();
  void HandleAccept(HttpConnection::Pointer new_connection,
                    const boost::system::error_code& error);
  TimeoutHandle SetTimer(std::chrono::milliseconds timeout,
                         TimerCallback callback,
                         HandleTimer handle_timer);
  void HandleTimeout(std::chrono::milliseconds /* timeout */,
                     TimeoutHandle timeout_handle,
                     TimerCallback callback,
                     const boost::system::error_code& error);
  void HandleInterval(std::chrono::milliseconds timeout,
                      TimeoutHandle interval_handle,
                      TimerCallback callback,
                      const boost::system::error_code& error);

  const unsigned short port_;
  boost::asio::io_service io_service_;
  boost::asio::ip::tcp::acceptor acceptor_;

  HttpConnection::UserFunc user_call_function_;
  TimersMap user_timers_;
};  // class ServerAsio

}  // namespace koohar

#endif  // koohar_server_asio_hh
