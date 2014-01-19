#ifndef koohar_request_hh
#define koohar_request_hh

#include <string>

#include "http_parser.hh"

namespace koohar {

class Request : public HttpParser {
public:
	int errorCode () const { return 400; }

	bool contains (const std::string& Url) const;
	bool corresponds (const std::string& StaticUrl) const;
}; // class Request

} // namespace koohar

#endif // koohar_request_hh
