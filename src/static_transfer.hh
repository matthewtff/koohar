#ifndef koohar_static_transfer_hh
#define koohar_static_transfer_hh

#include "request.hh"
#include "response.hh"
#include "server_config.hh"

namespace koohar {
class File;

class StaticTransfer {
 public:
  StaticTransfer(Request&& request,
                 Response&& response,
                 const ServerConfig& config);
  void Serve();

  static const StringMap& mime_types() { return mime_types_; }

 private:
  /**
   * If size of static file larger, it should be sent partially.
   * TODO(matthew): Make this constant configurable.
   */
  static const long unsigned int kMaxStaticSize = 16777216;  // 16 MB

  static const StringMap mime_types_;

  void SendAllFile(const File& file);
  void HandleError(const unsigned short Code);

  Request request_;
  Response response_;
  ServerConfig config_;
};  // class StaticTransfer

}  // namespace koohar

#endif  // koohar_static_transfer_hh
