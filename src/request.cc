#ifdef _WIN32

#include <winsock2.h>

#else /* _WIN32 */

#include <errno.h>

#endif /* _WIN32 */

#include <algorithm>
#include <cstring>

#include "request.hh"

#ifdef _DEBUG
#include <iostream>
#endif /* _DEBUG */

namespace koohar {

/* Converts a hex character to its integer value */
char from_hex(char ch) {
	return isdigit(ch) ? ch - '0' : tolower(ch) - 'a' + 10;
}

/* Returns a url-decoded version of str */
/* IMPORTANT: be sure to free() the returned string after use */
char *url_decode(char *str) {
	char *pstr = str, *buf = static_cast<char*>(malloc(strlen(str) + 1)), *pbuf = buf;
	while (*pstr) {
		if (*pstr == '%') {
			if (pstr[1] && pstr[2]) {
				*pbuf++ = from_hex(pstr[1]) << 4 | from_hex(pstr[2]);
				pstr += 2;
			}
		} else if (*pstr == '+') {
			*pbuf++ = ' ';
		} else {
			*pbuf++ = *pstr;
		}
		pstr++;
	}
	*pbuf = '\0';
	return buf;
}

Request::Request () : m_error_code(0), m_body_size(0), m_req_method_was(false),
	m_req_uri_was(false), m_req_version_was(false), m_req_header_name_was(false),
	m_req_read_body(false), m_req_body_accepted(0), m_req_first_string(true),
	m_req_body_multipart(false), m_req_body_multipart_file(false)
{
}

void Request::method (const std::string& NewMethod)
{
	m_method = NewMethod;
	std::transform(m_method.begin(), m_method.end(), m_method.begin(), tolower);
}

void Request::uri (const std::string& NewUri)
{
	m_uri = NewUri;
	char* decoded_uri = url_decode(const_cast<char*>(m_uri.c_str()));
	m_uri = decoded_uri;
	free(decoded_uri);
	size_t url_start = m_uri.find('/');
	size_t question_pos = m_uri.find('?');
	size_t url_end = (question_pos == m_uri.npos) ? m_uri.length() : question_pos;
	if ((url_start == m_uri.npos) || (url_end == m_uri.npos) || (url_start > url_end))
		return;
	m_url = m_uri.substr(url_start, url_end - url_start);
	parseUri(m_uri);
}

void Request::parseUri (const std::string& SomeUri, const char Sep, const char Eq)
{
	char* decoded_uri = url_decode(const_cast<char*>(SomeUri.c_str()));
	std::string temp_uri(decoded_uri);
	free(decoded_uri);
	size_t query_start = temp_uri.find('?');
	if (query_start != temp_uri.npos)
		temp_uri.erase(0, query_start + 1);
	if (!temp_uri.length())
		return;
	do {
		size_t eq_pos = temp_uri.find(Eq);
		size_t sep_pos = temp_uri.find(Sep);
		sep_pos = (sep_pos == temp_uri.npos) ? temp_uri.length() : sep_pos;
		m_body[temp_uri.substr(0, eq_pos)] = temp_uri.substr(eq_pos + 1, sep_pos - (eq_pos + 1));
		if (sep_pos == temp_uri.length())
			break;
		temp_uri.erase(0, sep_pos + 1);
	} while (true);
}

void Request::parseCookies (std::string SomeCookie)
{
	do {
		size_t eq_pos = SomeCookie.find('=');
		size_t sep_pos = SomeCookie.find(' ');
		sep_pos = (sep_pos == SomeCookie.npos) ? SomeCookie.length() : sep_pos;
		std::string cookie_name = SomeCookie.substr(0, eq_pos);
		m_cookies[cookie_name] = SomeCookie.substr(eq_pos + 1, sep_pos - (eq_pos + 2));
		if (sep_pos == SomeCookie.length())
			break;
		SomeCookie.erase(0, sep_pos + 1);
	} while (true);
}

std::string Request::body (const std::string& SomeName)
{
	if (m_body.find(SomeName) != m_body.end())
		return m_body[SomeName];
	else
		return std::string("");
}

void Request::header (std::string SomeName, std::string SomeValue)
{
	if (SomeValue[SomeValue.length() - 1] == '\r' || SomeValue[SomeValue.length() - 1] == '\n')
		SomeValue.erase(SomeValue.length() - 1, 1);
	std::transform(SomeName.begin(), SomeName.end(), SomeName.begin(), tolower);
	if (SomeName.compare("content-type")) // Content-Type header can contain boundary string is that case-sensitive
		std::transform(SomeValue.begin(), SomeValue.end(), SomeValue.begin(), tolower);
	if (!SomeName.compare("cookie"))
		parseCookies(SomeValue);
	else
		m_headers[SomeName] = SomeValue;
}

std::string Request::header (const std::string& SomeName)
{
	return (m_headers.find(SomeName) != m_headers.end())
		? m_headers[SomeName]
		: std::string();
}

std::string& Request::session (const std::string& Key)
{
	return (!m_session)
		? m_cookies[Key] // Dirty-dirty hack... need another workaround!
		: m_session->operator[](Key);
}

void Request::unsetSession (const std::string& Key)
{
	if (m_session)
		m_session->erase(Key);
}

bool Request::contains (const std::string& Param) const
{
	if (Param.empty() || m_url.empty())
		return false;
	if (m_url.find(Param) == m_url.npos)
		return false;
	return true;
}

bool Request::corresponds (const std::string& Param) const
{
	if (Param.empty() || m_url.empty() || m_url.length() < Param.length())
		return false;
	for (size_t count = 0; count < Param.length(); ++count)
		if (*(Param.c_str() + count) != *(m_url.c_str() + count))
			return false;
	return true;
}

bool Request::receive ()
{
	if (m_req_read_body)  {
		if (m_req_body_multipart) // accept multipart data
			return acceptMultipart();
		else // accept url encoded body
			return acceptBody();
	}
	char ch;
	do {
		int readed = 0;
		SocketErrorType sock_err = m_socket.getCh(&ch, 1, readed);
		switch (sock_err) {
			case SOCK_AGAIN_ERROR: return false;
			case SOCK_ERROR: return true;
			default:
				break;
		}
#ifdef _DEBUG
		std::cout << ch;
#endif /* _DEBUG */
		if (ch == ' ') {
			if (m_req_first_string) {
				if (!m_req_method_was) {
					m_method = m_req_method;
					m_req_method_was = true;
				}
				else if (!m_req_uri_was) {
					uri(m_req_uri);
					m_req_uri_was = true;
				}
			} else {
				if (!m_req_header_name_was) {
					m_req_header_name_was = true;
					m_req_header_name.erase(m_req_header_name.length() - 1, 1);
				} else
					m_req_header_value.append(&ch, 1);
			}
		} else if (ch == '\n') {
			if (m_req_first_string) {
				if (!m_req_version_was && !m_req_version.empty()) {
					char major[] = "a"; // 'a' is fairly chosen random char
					major[0] = m_req_version[m_req_version.length() - 4];
					char minor[] = "a";
					minor[0] = m_req_version[m_req_version.length() - 2];
					version(atoi(minor), atoi(major));
					m_req_version_was = true;
				}
				if (m_req_method_was && !m_req_version_was) { // We've got next string, but didn't receive version
					version(0, 9); // HTTP/0.9 does not send version info
					m_req_version_was = true;
				}
				m_req_first_string = false;
			} else {
				if (!m_req_header_name_was) { // It could be newline before POST body
					if (!m_method.compare("POST")) {
						m_req_read_body = true;
						analyzeBodyType();
						return receive();
					}
					return true;
				}
				// If not first line than we received one of headers
				header(m_req_header_name, m_req_header_value);
				m_req_header_name.erase();
				m_req_header_value.erase();
				m_req_header_name_was = false;
			}
		} else {
			if (m_req_first_string) {
				if (!m_req_method_was)
					m_req_method.append(&ch, 1);
				else if (!m_req_uri_was)
					m_req_uri.append(&ch, 1);
				else if (!m_req_version_was)
					m_req_version.append(&ch, 1);
			} else {
				if (!m_req_header_name_was)
					m_req_header_name.append(&ch, 1);
				else
					m_req_header_value.append(&ch, 1);
			}
		}
	} while (true);
	return true;
}

void Request::clear ()
{
	m_error_code = 0;
	m_method = "";
	m_uri = "";
	m_url = "";
	m_body.erase(m_body.begin(), m_body.end());
	m_headers.erase(m_headers.begin(), m_headers.end());
	m_version.m_major = m_version.m_minor = 0;
	
	m_req_method_was = false;
	m_req_method = "";
	m_req_uri_was = false;
	m_req_uri = "";
	m_req_version_was = false;
	m_req_version = "";
	m_req_header_name_was = false;
	m_req_header_name = "";
	m_req_header_value = "";
	m_req_read_body = false;
	m_req_body_accepted = 0;
	m_req_body = "";
	m_req_first_string = false;
}

#ifdef _DEBUG
void Request::print ()
{
	std::cout << "Error code: " << m_error_code << std::endl;
	std::cout << "IP: " << m_socket.ip() << std::endl;
	std::cout << "Port: " << m_socket.port() << std::endl;
	std::cout << "Method: " << m_method << std::endl;
	std::cout << "URI: " << m_uri << std::endl;
	std::cout << "URL: " << m_url << std::endl;
	for (auto anyPar = m_body.begin(); anyPar != m_body.end(); anyPar++)
		std::cout << anyPar->first << " = " << anyPar->second << std::endl;
	std::cout << "Version: HTTP/" << m_version.m_major << "." << m_version.m_minor << std::endl;
	std::cout << "Headers:" << std::endl;
	for (auto anyHeader = m_headers.begin(); anyHeader != m_headers.end(); anyHeader++)
		std::cout << anyHeader->first << " : " << anyHeader->second << std::endl;
	std::cout << std::endl;
}
#endif /* _DEBUG */

// private methods

void Request::analyzeBodyType()
{
	m_body_size = atoi(m_headers["content-length"].c_str());
	if (m_body_size <= 0) {
		m_error_code = 411; // Length Required
#ifdef _DEBUG
		std::cout << 411 << std::endl;
#endif /* _DEBUG */
		return;
	}
	std::string& content_type = m_headers["content-type"];
	size_t semicolon_pos = content_type.find("; ");
	if (semicolon_pos == content_type.npos) {
		return; // post:www-url-encoded
	}
	std::string enctype = content_type.substr(0, semicolon_pos);
#ifdef _DEBUG
	std::cout << "Enctype : " << enctype << std::endl;
#endif /* _DEBUG */
	if (!enctype.compare("multipart/form-data")) {
		size_t boundary_pos = content_type.find("boundary=");
		if (boundary_pos == content_type.npos) {
			m_error_code = 400; // Bad Request
#ifdef _DEBUG
			std::cout << "multipart:400" << std::endl;
#endif /* _DEBUG */
			return;
		}
		m_boundary = content_type.substr(boundary_pos + 9); // 9 is length of string 'boundary='
#ifdef _DEBUG
		std::cout << "Multipart!" << std::endl;
#endif /* _DEBUG */
		resetBodyMultipartVariables ();
	}
}

bool Request::acceptBody()
{
	int msg_size = m_body_size - m_req_body_accepted;
	if (msg_size <= 0) // really something bad happened
		return true;
	char* post_body = new char[msg_size + 1]; // 1 is for '\0'
	size_t accepted = 0;
	do {
		int readed = 0;
		SocketErrorType sock_err = m_socket.getCh(&post_body[accepted], msg_size - accepted, readed);
		switch (sock_err) {
			case SOCK_AGAIN_ERROR: // this error means for now there is no data in socket
				post_body[accepted] = '\0'; // so we save all we got, and return false
				m_req_body.append(post_body); // to show that we'll wait some time for data
				m_req_body_accepted += accepted;
				delete []post_body;
			return false;
			case SOCK_ERROR:
				delete []post_body;
				m_error_code = 400; // Bad Request
			return true; // something bad happened
			default:
				break;
		}
		accepted += readed;
		if (accepted == static_cast<size_t>(msg_size) || readed == 0) { // we've got all we need
			post_body[accepted] = '\0';
			m_req_body.append(post_body);
#ifdef _DEBUG
			std::cout << m_req_body << std::endl;
#endif /* _DEBUG */
			delete []post_body;
			parseUri(m_req_body.c_str());
			return true;
		}
	} while (true);
	delete []post_body;
	return true;
}

// Accepting headers for each input on form here
// But their value in another method
bool Request::acceptMultipart()
{
#ifdef _DEBUG
	std::cout << "Accepting multipart..." << std::endl;
#endif /* _DEBUG */
	int msg_size = m_body_size - m_req_body_accepted;
	if (msg_size <= 0) // really something bad happened
		return true;
	size_t accepted = 0;
	do {
		int readed = 0;
		char ch;
		SocketErrorType sock_err = m_socket.getCh(&ch, 1, readed);
#ifdef _DEBUG
		std::cout << ch;
#endif /* _DEBUG */
		switch (sock_err) {
			case SOCK_AGAIN_ERROR:
				return false;
			case SOCK_ERROR:
				return true;
			default:
				break;
		}
		accepted += readed;
		if (ch == '\n') {
			if (m_req_first_string) {
				m_req_first_string = false;
				if (m_req_body_multipart_header.empty() || !checkBoundary(m_req_body_multipart_header))
					return true; // some error actualy
			} else {
				if (m_req_header_name_was) {
					m_req_header_name.erase(m_req_header_name.length() - 1, 1);
					m_req_headers[m_req_header_name] = m_req_header_value;
					m_req_header_name.erase();
					m_req_header_value.erase();
					m_req_header_name_was = false;
				} else {
					if (acceptMultipartFile(accepted)) // error occured while accepting multipart file
						return true;
				}
			}
		} else if (ch == ' ') {
			if (m_req_header_name_was) {
				m_req_header_value.append(&ch, 1);
			} else {
				m_req_header_name_was = true;
			}
		} else {
			if (m_req_first_string)
				m_req_body_multipart_header.append(&ch, 1);
			else {
				if (m_req_header_name_was)
					m_req_header_value.append(&ch, 1);
				else
					m_req_header_name.append(&ch, 1);
			}
		}
		if (accepted == static_cast<size_t>(msg_size) || readed == 0) {
#ifdef _DEBUG
			std::cout << "Got all we need!..." << std::endl;
#endif /* _DEBUG */
			return true;
		}
	} while (true);
}

bool Request::acceptMultipartFile (size_t& accepted)
{
#ifdef _DEBUG
	std::cout << "Parsed multipart headers: " << std::endl;
	for (auto it = m_req_headers.begin(); it != m_req_headers.end(); ++it) {
		std::cout << it->first << " : " << it->second << std::endl;
	}
#endif /* _DEBUG */
	do {
		int readed = 0;
		char ch;
		SocketErrorType sock_err = m_socket.getCh(&ch, 1, readed);
#ifdef _DEBUG
		std::cout << ch;
#endif /* _DEBUG */
		switch (sock_err) {
			case SOCK_AGAIN_ERROR:
				//resetBodyMultipartVariables();
				return false;
			case SOCK_ERROR:
				resetBodyMultipartVariables();
				return true;
			default:
				break;
		}
		accepted += readed;
	} while (true);
	resetBodyMultipartVariables();
	return true;
}

void Request::resetBodyMultipartVariables ()
{
	m_req_body_multipart = true;
	m_req_first_string = true; // Will be used to detect if it boundary string
	m_req_header_name_was = false;
	m_req_body_multipart_file = false;
	m_req_header_name.erase();
	m_req_header_value.erase();
	m_req_headers.clear();
}

bool Request::checkBoundary (const std::string& StrToCheck, bool PreDelimeter)
{
#ifdef _DEBUG
	std::cout << "[Request::checkBoundary] StrToCheck : " << StrToCheck << std::endl;
#endif /* _DEBUG */
	std::string Needle("--");
	Needle.append(m_boundary).append(PreDelimeter ? "" : "--");
#ifdef _DEBUG
	std::cout << "[Request::checkBoundary] NeedleStr  : " << Needle << std::endl;
	std::cout << "[Request::checkBoundary] Return : " << StrToCheck.compare(Needle) << std::endl;
#endif /* _DEBUG */
	return StrToCheck.compare(Needle) > 0;
}

}; // namespace koohar
