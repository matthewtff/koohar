#include "client_cache.hh"

#include "base/utils.hh"

namespace koohar {

namespace {

const char kEtagHeader[] = "etag";
const char kIfNoneMatchHeader[] = "If-None-Match";

}  // anonymous namespace

void ClientCache::SetCacheEntry(const ClientRequest& request,
                                const ClientResponse& response) {
  cache_map_.insert(std::make_pair(request.url(), response));
}

ClientResponse ClientCache::PrepareRequest(ClientRequest& request) {
  CacheMap::iterator cache_pair = cache_map_.find(request.url());
  if (cache_pair == cache_map_.cend())
    return ClientResponse();
  const std::string etag_value = cache_pair->second.Header(kEtagHeader);
  if (!etag_value.empty()) {
    request.SetHeader(kIfNoneMatchHeader, etag_value);
  }
  return cache_pair->second;
}

}  // namespace koohar
