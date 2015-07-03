#ifndef koohar_http_hh
#define koohar_http_hh

#include <string>

namespace koohar {
namespace HTTP {

extern const char kHeaderDelimiter[3];
extern const char kLineDelimiter[3];

struct Version {
  unsigned short int major_;
  unsigned short int minor_;
};

enum class Method {
  Options,
  Get,
  Head,
  Post,
  Put,
  Delete,
  Trace,
  Connect,
};

std::string MethodToString(const HTTP::Method method);
std::string VersionToString(const Version version);

}  // namespace HTTP
}  // namespace koohar

#endif  // koohar_http_hh
