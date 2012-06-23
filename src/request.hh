#ifndef koohar_request_hh
#define koohar_request_hh

#include <string>
#include <map>

#include "socket.hh"
#include "file.hh"

namespace koohar {

struct Version {
	short int m_major;
	short int m_minor;
}; // struct Version

class Request {

public:
	enum { BUF_SIZE = 2048 };
	typedef std::map<std::string, std::string> StringMap;
	typedef std::map<std::string, File> FileMap;

public:
	Request ();
	void socket (const Socket& NewSocket) { m_socket = NewSocket; }
	Socket socket () const { return m_socket; }
	void method (const std::string& NewMethod);
	std::string& method () { return m_method; }
	void uri (const std::string& NewUri);
	std::string& uri () { return m_uri; }
	std::string& url () { return m_url; }
	void parseUri (const std::string& SomeUri, const char Sep = '&', const char Eq = '=');
	void parseCookies (std::string SomeCookie);
	std::string body (const std::string& SomeName);
	void version (const short int NewMajor, const short int NewMinor) { m_version.m_major = NewMajor; m_version.m_minor = NewMinor; }
	Version& version () { return m_version; }
	void header (std::string NewName, std::string NewValue);
	std::string header (const std::string& SomeName);
	StringMap& cookies () { return m_cookies; }
	std::string cookie (const std::string& CookieName) { return m_cookies[CookieName]; }
	void session (StringMap* Session) { m_session = Session; }
	std::string& session (const std::string& Key);
	void unsetSession (const std::string& Key);
	std::string ip () const { return m_socket.ip(); }
	std::string port () const { return m_socket.port(); }
	int errorCode () const { return m_error_code; }
	bool contains (const std::string& Param) const; // check if url contains substr
	bool corresponds (const std::string& Param) const; // check if url starts with substr
	bool receive();
	void clear();
#ifdef _DEBUG
	void print();
#endif /* _DEBUG */

private:
	Socket m_socket;
	std::string m_method;
	std::string m_uri;
	std::string m_url;
	StringMap m_body;
	StringMap m_headers;
	StringMap m_cookies;
	StringMap* m_session;
	Version m_version;
	int m_error_code; // Should be set only if some error occured
	int m_body_size;
	std::string m_boundary;

	// we need to keep this state
	// cause receive method can be called multiple times (non-block i/o)
	bool m_req_method_was;
	std::string m_req_method;
	bool m_req_uri_was;
	std::string m_req_uri;
	bool m_req_version_was;
	std::string m_req_version;
	bool m_req_header_name_was;
	std::string m_req_header_name, m_req_header_value;
	bool m_req_read_body;
	size_t m_req_body_accepted;
	std::string m_req_body;
	bool m_req_first_string;
	StringMap m_req_headers; // multipart headers

	bool m_req_body_multipart;
	bool m_req_body_multipart_file;
	std::string m_req_body_multipart_header; // contains multipart header or boundary string

private:
	void analyzeBodyType ();
	bool acceptBody ();
	bool acceptMultipart ();
	bool acceptMultipartFile (size_t& accepted);
	void resetBodyMultipartVariables ();
	bool checkBoundary (const std::string& StrToCheck, bool PreDelimeter = true);
}; // class Request

}; // namespace koohar

#endif // koohar_request_hh
