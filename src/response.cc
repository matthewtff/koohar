#include <cstring>
#include <errno.h>

#include "response.hh"

#ifdef _DEBUG

#include <iostream>

#endif

namespace koohar {

Response::Response (Sender& sender) : m_headers_allowed(true), m_sender(sender)
{
}

void Response::writeHead(unsigned short State)
{
	if (!m_headers_allowed)
		return;
	std::string head("HTTP/1.1 ");
	head.append(States[State]);
	head.append("\r\n");
	transfer(head.c_str(), head.length());
}

void Response::header(const std::string& HeaderName, const std::string& HeaderValue, bool Replace)
{
	if (!m_headers_allowed || HeaderName.empty() || HeaderValue.empty())
		return;
	std::string& hdr = m_headers[HeaderName];
	hdr = (hdr.empty() || Replace)
		? HeaderValue
		: (hdr.append("; ").append(HeaderValue));
}

/**
 * Actually you should be very carefull to this method cause it does not seem to replace existing cookie value.
 */
bool Response::cookie(const std::string& CookieName, const std::string& CookieValue)
{
	if (CookieName.empty() || CookieValue.empty())
		return false;
	header("Set-Cookie", CookieName + "=" + CookieValue, false); // we do not want to replace all cookies, just add one
	return true;
}

bool Response::body(const void* Buffer, const off_t BufferSize)
{
	if (m_headers_allowed) { // sending headers
		sendHeaders();
	} else if (!m_headers.empty()) { // TODO: fix problem with headers by good error handling
#ifdef _DEBUG
		std::cerr << "[Response::body] Double header sending" << std::endl;
#endif
		return false;
	}
	return transfer(Buffer, BufferSize);
}

bool Response::sendFile(const char* FileName, const off_t Size, const off_t Offset)
{
	sendHeaders();
	return m_sender.sendFile(m_socket.fd(), FileName, Size, Offset);
}

void Response::end(const void* Buffer, const off_t BufferSize)
{
	if (Buffer && BufferSize)
		body(Buffer, BufferSize);
	else if (m_headers_allowed)
		sendHeaders();
	m_sender.close(m_socket.fd());
}

void Response::redirect(const std::string& Url)
{
	header("Location", Url); // Redirecting with HTTP header
	writeHead(302);
}

void Response::clear()
{
	m_headers_allowed = true;
	m_headers.erase(m_headers.begin(), m_headers.end());
}

// private methods

bool Response::transfer(const void* Buffer, const off_t BufferSize)
{
	return m_sender.send(m_socket.fd(), static_cast<const char*>(Buffer), BufferSize);
}

void Response::sendHeaders()
{
	for(StringMap::iterator anyHeader = m_headers.begin(); anyHeader != m_headers.end(); anyHeader++) { // send all headers
		std::string str(anyHeader->first);
		str.append(": ");
		str.append(anyHeader->second);
		str.append("\r\n");
		transfer(str.c_str(), str.length());
	}
	transfer(std::string("\r\n").c_str(), 2);
	m_headers.clear();
	m_headers_allowed = false; // no more headers
}

bool initStates (Response::StateMap& state_map)
{
	state_map[100] = "100 Continue";
	state_map[101] = "101 Switching Protocols";
	state_map[102] = "102 Processing";

	state_map[200] = "200 OK";
	state_map[201] = "201 Created";
	state_map[202] = "202 Accepted";
	state_map[203] = "203 Non-Authoritative Information";
	state_map[204] = "204 No Content";
	state_map[205] = "205 Reset Content";
	state_map[206] = "206 Partial Content";
	state_map[207] = "207 Multi-Status";
	state_map[226] = "226 IM Used";

	state_map[300] = "300 Multiple Choises";
	state_map[301] = "301 Moved Permamently";
	state_map[302] = "302 Found";
	state_map[303] = "303 See Other";
	state_map[304] = "304 Not Modified";
	state_map[305] = "305 Use Proxy";
	state_map[307] = "307 Temporary Redirect";

	state_map[400] = "400 Bad Request";
	state_map[401] = "401 Unauthorized";
	state_map[402] = "402 Payment Required";
	state_map[403] = "403 Forbidden";
	state_map[404] = "404 Not Found";
	state_map[405] = "405 Method Not Allowed";
	state_map[406] = "406 Not Acceptable";
	state_map[407] = "407 Proxy Authentication Required";
	state_map[408] = "408 Request Timeout";
	state_map[409] = "409 Conflict";
	state_map[410] = "410 Gone";
	state_map[411] = "411 Length Required";
	state_map[412] = "412 Precondition Failed";
	state_map[413] = "413 Request Entity Too Large";
	state_map[414] = "414 Request-URI Too Long";
	state_map[415] = "415 Unsupported Media Type";
	state_map[416] = "416 Requested Range Not Satisfiable";
	state_map[417] = "417 Expectation Failed";
	state_map[418] = "418 I'm a teapot"; // First april joke : rfc 2424 ( http://tools.ietf.org/html/rfc2324 )
	state_map[422] = "422 Unprocessable Entity";
	state_map[423] = "423 Loked";
	state_map[424] = "424 Failed Dependency";
	state_map[425] = "425 Unordered Collection";
	state_map[426] = "426 Upgrade Required";
	state_map[449] = "449 Retry Width";
	state_map[456] = "456 Unrecoverable Error";

	state_map[500] = "500 Internal Server Error";
	state_map[501] = "501 Not Implemented";
	state_map[502] = "502 Bad Gateway";
	state_map[503] = "503 Service Unavailable";
	state_map[504] = "504 Gateway Timeout";
	state_map[505] = "505 HTTP Version No Supported";
	state_map[506] = "506 Variant Also Negotiates";
	state_map[507] = "507 Insufficient Storage";
	state_map[509] = "509 Bandwidth Limit Exceeded";
	state_map[510] = "510 Not Extended";
	
	return true;
}

}; // namespace koohar
