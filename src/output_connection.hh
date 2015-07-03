#ifndef koohar_output_connection_hh
#define koohar_output_connection_hh

#include <list>
#include <memory>
#include <map>
#include <string>

#include <boost/asio.hpp>

#include "client_request.hh"
#include "client_response.hh"

namespace koohar {

class OutputConnection : public std::enable_shared_from_this<OutputConnection> {
 public:
  static const unsigned int kMaxResponseSize = 65536;  // 64KB

  typedef std::shared_ptr<OutputConnection> Pointer;
  typedef std::function<void(ClientRequest&&, ClientResponse&&)> Callback;

  static void Run(boost::asio::io_service& io_service,
                  ClientRequest&& request,
                  Callback callback,
                  boost::asio::ip::tcp::resolver::iterator hosts);

 private:
  OutputConnection(boost::asio::io_service& io_service,
                   ClientRequest&& request,
                   Callback callback);
  void Start(boost::asio::ip::tcp::resolver::iterator hosts);
  void Respond();
  void Write();
  void Read();
  void HandleConnect(const boost::system::error_code& error,
                     boost::asio::ip::tcp::resolver::iterator);
  void HandleRead(const boost::system::error_code& error,
                  const std::size_t bytes_transferred);
  void HandleWrite(const boost::system::error_code& error,
                   const std::size_t bytes_transferred);

  boost::asio::ip::tcp::socket socket_;
  std::string request_text_;
  size_t bytes_sent_;
  char response_buffer_[kMaxResponseSize];
  ClientRequest request_;
  ClientResponse response_;
  Callback callback_;
}; // class OutputConnection

} // namespace koohar

#endif // koohar_output_connection_hh
