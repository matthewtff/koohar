#include "response.hh"

#include <algorithm>
#include <cstring>
#include <errno.h>

#include "file.hh"
#include "filemapping.hh"

#ifdef _DEBUG
#include <iostream>
#endif

namespace koohar {

Response::StateMap Response::States = Response::initStates();

/*Response::Response (Sender& sender) : m_headers_allowed(true), m_sender(sender)
{
}*/

Response::Response (HttpConnection::Pointer Connection) :
	m_headers_allowed(true), m_connection(Connection)
{}

void Response::writeHead(unsigned short State)
{
	if (!m_headers_allowed)
		return;
	std::string head("HTTP/1.1 ");
	head += States[State] + "\r\n";
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
 * Actually you should be very carefull to this method cause it does not seem
 * to replace existing cookie value.
 */
bool Response::cookie(const std::string& CookieName, const std::string& CookieValue)
{
	std::string cookie_str = CookieName + "=" + CookieValue;
	cookie_str.append("; Path=/;");
	if (CookieName.empty() || CookieValue.empty())
		return false;
	header("Set-Cookie", cookie_str, false); // dont replace all cookies, just add
	return true;
}

bool Response::body (const std::string& String)
{
	return body(String.c_str(), String.length());
}

bool Response::body(const void* Buffer, const off_t BufferSize)
{
	if (m_headers_allowed) { // sending headers
		sendHeaders();
	} else if (!m_headers.empty()) {
#ifdef _DEBUG
		std::cout << "[Response::body] Double header sending" << std::endl;
#endif
		return false;
	}
	return transfer(Buffer, BufferSize);
}

bool Response::sendFile(const char* FileName, const off_t Size, const off_t Offset)
{
	if (m_headers_allowed)
		sendHeaders();

	File file(FileName);
	if (!file.open(File::ReadOnly))
		return false;
	
	FileMapping mapping(file.getHandle());
	char* mapped_file = mapping.map(Size, Offset);

	if (mapped_file)
		transfer(mapped_file, Size);
	else
		return false;
	mapping.unMap();

	return true;
}

void Response::end(const std::string& String)
{
	return end(String.c_str(), String.length());
}

void Response::end(const void* Buffer, const off_t BufferSize)
{
	if (m_headers_allowed)
		sendHeaders();
	if (Buffer && BufferSize)
		body(Buffer, BufferSize);
	m_connection->close();
}

void Response::end()
{
	if (m_headers_allowed)
		sendHeaders();
	m_connection->close();
}

void Response::redirect(const std::string& Url)
{
	writeHead(302);
	header("Location", Url); // Redirecting with HTTP header
}

Response::StateMap Response::initStates ()
{
	StateMap States;
	States[100] = "100 Continue";
	States[101] = "101 Switching Protocols";
	States[102] = "102 Processing";

	States[200] = "200 OK";
	States[201] = "201 Created";
	States[202] = "202 Accepted";
	States[203] = "203 Non-Authoritative Information";
	States[204] = "204 No Content";
	States[205] = "205 Reset Content";
	States[206] = "206 Partial Content";
	States[207] = "207 Multi-Status";
	States[226] = "226 IM Used";

	States[300] = "300 Multiple Choises";
	States[301] = "301 Moved Permamently";
	States[302] = "302 Found";
	States[303] = "303 See Other";
	States[304] = "304 Not Modified";
	States[305] = "305 Use Proxy";
	States[307] = "307 Temporary Redirect";

	States[400] = "400 Bad Request";
	States[401] = "401 Unauthorized";
	States[402] = "402 Payment Required";
	States[403] = "403 Forbidden";
	States[404] = "404 Not Found";
	States[405] = "405 Method Not Allowed";
	States[406] = "406 Not Acceptable";
	States[407] = "407 Proxy Authentication Required";
	States[408] = "408 Request Timeout";
	States[409] = "409 Conflict";
	States[410] = "410 Gone";
	States[411] = "411 Length Required";
	States[412] = "412 Precondition Failed";
	States[413] = "413 Request Entity Too Large";
	States[414] = "414 Request-URI Too Long";
	States[415] = "415 Unsupported Media Type";
	States[416] = "416 Requested Range Not Satisfiable";
	States[417] = "417 Expectation Failed";
	States[418] = "418 I'm a teapot"; // First april joke : rfc 2424 ( http://tools.ietf.org/html/rfc2324 )
	States[422] = "422 Unprocessable Entity";
	States[423] = "423 Loked";
	States[424] = "424 Failed Dependency";
	States[425] = "425 Unordered Collection";
	States[426] = "426 Upgrade Required";
	States[449] = "449 Retry Width";
	States[456] = "456 Unrecoverable Error";

	States[500] = "500 Internal Server Error";
	States[501] = "501 Not Implemented";
	States[502] = "502 Bad Gateway";
	States[503] = "503 Service Unavailable";
	States[504] = "504 Gateway Timeout";
	States[505] = "505 HTTP Version No Supported";
	States[506] = "506 Variant Also Negotiates";
	States[507] = "507 Insufficient Storage";
	States[509] = "509 Bandwidth Limit Exceeded";
	States[510] = "510 Not Extended";
	
	return States;
}

// private

bool Response::transfer(const void* Buffer, const off_t BufferSize)
{
	m_connection->write(static_cast<const char*>(Buffer), BufferSize);
	return true;
}

void Response::sendHeaders()
{
	std::for_each(m_headers.begin(), m_headers.end(), [&](const std::pair<std::string, std::string>& header) {
		transfer(header.first.c_str(), header.first.length());
		transfer(": ", 2);
		transfer(header.second.c_str(), header.second.length());
		transfer("\r\n", 2);
	});
	transfer("\r\n", 2);
	m_headers.clear();
	m_headers_allowed = false; // no more headers
}


}; // namespace koohar
