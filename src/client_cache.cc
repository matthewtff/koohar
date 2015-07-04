#include "client_cache.hh"

#include "base/utils.hh"

namespace koohar {

void ClientCache::SetCacheEntry(const ClientRequest& request,
                                const ClientResponse& response) {
  cache_map_.insert(std::make_pair(request.url(), response));
}

ClientResponse ClientCache::PrepareRequest(ClientRequest& request) {
  CacheMap::iterator cache_pair = cache_map_.find(request.url());
  if (cache_pair == cache_map_.cend()) {
    return ClientResponse();
  }
  const std::string etag_value = cache_pair->second.Header("etag");
  if (!etag_value.empty()) {
    request.SetHeader("If-None-Match", etag_value);
  }
  return cache_pair->second;
}

}  // namespace koohar
