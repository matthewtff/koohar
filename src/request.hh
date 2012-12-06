#ifndef koohar_request_hh
#define koohar_request_hh

#include <string>

#include "http_parser.hh"

namespace koohar {


class Request : public HttpParser {
public:
	void session (StringMap* Session) { m_session = Session; }
	std::string& session (const std::string& Key);
	void unsetSession (const std::string& Key);
	int errorCode () const { return 400; }

	bool contains (const std::string& Url) const;
	bool corresponds (const std::string& StaticUrl) const;

	static std::string m_session_error;

private:
	StringMap* m_session;
}; // class Request

} // namespace koohar

#endif // koohar_request_hh
