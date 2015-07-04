#ifndef koohar_client_cache_hh
#define koohar_client_cache_hh

#include "client_request.hh"
#include "client_response.hh"

namespace koohar {

class ClientCache {
 public:
  void SetCacheEntry(const ClientRequest& request,
                     const ClientResponse& response);
  ClientResponse PrepareRequest(ClientRequest& request);

 private:
  using CacheMap = std::map<std::string, ClientResponse>;
  CacheMap cache_map_;
};  // class ClientCache

}  // namespace koohar

#endif  // koohar_client_cache_hh
