#ifndef koohar_request_hh
#define koohar_request_hh

#include <string>
#include <map>

#include "socket.hh"
#include "file.hh"
#include "http_parser.hh"

namespace koohar {


class Request : public HttpParser {

public:
	enum { BUF_SIZE = 2048 };
	typedef std::map<std::string, File> FileMap;

public:
	Request ();
	void socket (const Socket& NewSocket) { m_socket = NewSocket; }
	Socket socket () const { return m_socket; }
	void session (StringMap* Session) { m_session = Session; }
	std::string& session (const std::string& Key);
	void unsetSession (const std::string& Key);
	bool contains (const std::string& Param) const; // check if url contains substr
	bool corresponds (const std::string& Param) const; // check if url starts with substr
	bool receive();
	int errorCode () const { return 400; }

	static std::string m_session_error;

private:
	Socket m_socket;
	StringMap* m_session;
}; // class Request

}; // namespace koohar

#endif // koohar_request_hh
