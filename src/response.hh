#ifndef koohar_response_hh
#define koohar_response_hh

#include <string>
#include <map>

#include "sender.hh"
#include "socket.hh"

namespace koohar {

typedef std::map<std::string, std::string> StringMap;

class Response {

public:
	typedef std::map<unsigned short, std::string> StateMap;

public:
	Response(Sender& sender); // Response is useless whitout Sender.
	void socket(const Socket& socket) { m_socket = socket; }

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

	/**
	 * Send some data. Can be called multiple times.
	 */
	bool body(const void* Buffer, const off_t BufferSize);

	/**
	 * Cannot be used whith body() method. After using you still should call
	 * the end() method.
	 */
	bool sendFile(const char* FileName, const off_t Size, const off_t Offset);

	/**
	 * Terminates connection to client.
	 */
	void end(const void* Buffer = NULL, const off_t BufferSize = 0);

	/**
	 * Sends redirecting header and 302 (Redirect) status. You stll can send
	 * some data using body() method and even set additional headers. Do not
	 * forget to close connection using end().
	 */
	void redirect (const std::string& Url);
	void clear(); // No idea...

private:
	Socket m_socket;
	StringMap m_headers;
	bool m_headers_allowed; // True, till any part of body is sent.
	Sender& m_sender;

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
}; // class Response

/**
 * States are very often used, so no need to create(allocate) that much
 * memory every time response object created. Thats why static map is created.
 */
bool initStates (Response::StateMap& state_map);

static Response::StateMap States;
static bool __dummy = initStates(States); // Created to call function once.

}; // namespace koohar

#endif // koohar_response_hh
