#ifndef koohar_response_hh
#define koohar_response_hh

#include <string>
#include <map>

#include "http_connection.hh"
#include "file.hh"
#include "json.hh"

namespace koohar {

class Sender;

typedef std::map<std::string, std::string> StringMap;

class Response {
public:
	typedef std::map<unsigned short, std::string> StateMap;

	static StateMap States;

public:
	explicit Response(HttpConnection::Pointer Connection);

	/**
	 * Sets and sends appropriate http status to client.
	 */
	void writeHead(const unsigned short State);

	/**
	 * Sets header. Can be called multiple times. If Replace set to false, the
	 * value whould be added using semicolon (or how do call that ';' ?).
	 */
	void header(const std::string& HeaderName, const std::string& HeaderValue,
		bool Replace = true);

	/**
	 * Calls header() method using 'Set-Cookie' HeaderName. Be carefull on using.
	 */
	bool cookie (const std::string& CookieName, const std::string& CookieValue);

	bool body (const std::string& String);

	/**
	 * Send some data. Can be called multiple times.
	 */
	bool body(const void* Buffer, const off_t BufferSize);

	/**
	 * Cannot be used whith body() method. After using you still should call
	 * the end() method.
	 */
	bool sendFile(File::Handle FileHandle, const off_t Size, const off_t Offset);

	void end (const std::string& String);

	/**
	 * Terminates connection to client.
	 */
	void end(const void* Buffer, const off_t BufferSize);

	void end();

	/**
	 * Sends redirecting header and 302 (Redirect) status. You stll can send
	 * some data using body() method and even set additional headers. Do not
	 * forget to close connection using end().
	 */
	void redirect (const std::string& Url);

	void sendJSON (const JSON::Object& Obj);

	void badRequest ();

	/**
	 * States are very often used, so no need to create(allocate) that much
	 * memory every time response object created. Thats why static map is created.
	 */
	static StateMap initStates ();


private:
	/**
	 * body() and end() methods have some identical code, so it moved to
	 * private method.
	 */
	bool transfer(const void* Buffer, const off_t BufferSize);

	/**
	 * This method handle '\n\r' sending after headers part and setting
	 * appropriate value to m_headers_allowed variable.
	 */
	void sendHeaders();

private:
	StringMap m_headers;
	bool m_headers_allowed; // True, till any part of body is sent.
	
	HttpConnection::Pointer m_connection;
}; // class Response

}; // namespace koohar

#endif // koohar_response_hh
