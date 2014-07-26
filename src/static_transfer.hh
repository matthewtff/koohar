#ifndef koohar_static_transfer_hh
#define koohar_static_transfer_hh

#include "request.hh"
#include "response.hh"
#include "server_config.hh"

namespace koohar {

class StaticTransfer {
 public:
  StaticTransfer(Request&& request,
                 Response&& response,
                 const ServerConfig& config);
  void Serve();

 public:
  static const StringMap mime_types_;

 private:
  /**
   * If size of static file large then this it partially.
   * TODO(matthew): Make this constant configurable.
   */
  static const unsigned int MaxStaticSize = 16777216; // 16 MB

  bool isVulnerable(const std::string& FileName);
  void handleError(const unsigned short Code);
  std::string MimeFromName(const std::string& FileName);

 private:
  Request m_request;
  Response m_response;
  ServerConfig m_config;
};  // class StaticTransfer

}  // namespace koohar

#endif  // koohar_static_transfer_hh