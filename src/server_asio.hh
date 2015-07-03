#ifndef koohar_server_asio_hh
#define koohar_server_asio_hh

#include <chrono>
#include <functional>
#include <unordered_map>

#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>

#include "client_request.hh"
#include "client_response.hh"
#include "input_connection.hh"
#include "server_config.hh"

namespace koohar {

class ServerAsio : public ServerConfig {
 public:
  using TimerCallback = std::function<void()>;
  using TimeoutHandle = std::size_t;
  using ClientRequestCallback =
      std::function<void(ClientRequest, ClientResponse)>;
  ServerAsio(const unsigned short Port = 80);

  void Listen(InputConnection::UserFunc user_call_function);
  void Stop() { io_service_.stop(); }

  unsigned short port() const { return port_; }

  TimeoutHandle SetTimeout(std::chrono::milliseconds timeout,
                           TimerCallback callback);
  TimeoutHandle SetInterval(std::chrono::milliseconds timeout,
                            TimerCallback callback);
  void ClearTimeout(TimeoutHandle timeout_handle);
  void ClearInterval(TimeoutHandle interval_handle);

  void MakeClientRequest(ClientRequest&& client_request,
                         ClientRequestCallback client_request_callback);

 private:
  using TimersMap =
      std::unordered_map<TimeoutHandle, boost::asio::steady_timer>;
  using HandleTimer =
      void (ServerAsio::*)(std::chrono::milliseconds,
                           TimeoutHandle,
                           TimerCallback,
                           const boost::system::error_code&);
  using Resolver = boost::asio::ip::tcp::resolver;
  bool Accept();
  void HandleAccept(InputConnection::Pointer new_connection,
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
  void HandleResolve(ClientRequest& client_request,
                     ClientRequestCallback client_request_callback,
                     const boost::system::error_code& error,
                     Resolver::iterator iterator);

  const unsigned short port_;
  boost::asio::io_service io_service_;
  boost::asio::ip::tcp::acceptor acceptor_;
  Resolver resolver_;

  InputConnection::UserFunc user_call_function_;
  TimersMap user_timers_;
};  // class ServerAsio

}  // namespace koohar

#endif  // koohar_server_asio_hh
