#include "http_parser.hh"

#include <cctype>
#include <cstdlib>

#include <iostream>

namespace koohar {

inline HttpParser::Method operator++ (HttpParser::Method& rs)
{
	return rs = static_cast<HttpParser::Method>(rs + 1);
}

boost::regex HttpParser::m_cookie_regex
	("([^=]+)=?([^;]*)(;)?[:space]?");

HttpParser::StateCallback HttpParser::m_callbacks[] = {
	&HttpParser::parseMethod,
	&HttpParser::parseUri,
	&HttpParser::parseHttpVersion,
	&HttpParser::parseHeaderName,
	&HttpParser::parseHeaderValue,
	&HttpParser::parseBody
};

/**
 * Method token MUST be upper case.
 */
const char* HttpParser::m_method_strings[] = {
	"OPTIONS",
	"GET",
	"HEAD",
	"POST",
	"PUT",
	"DELETE",
	"TRANCE",
	"CONNECT"
};

HttpParser::HttpParser () : m_state(OnMethod), m_content_length(0)
{}

bool HttpParser::update (const char* Data, unsigned int Size)
{
	for (unsigned int counter = 0; counter < Size; ++counter) {
		if (m_state == OnParseError)
			return false;
		if (m_token.length() > MaxTokenSize || !Data[counter]) {
			m_state = OnParseError;
			return false; // Prevent buffer overflow.
		}
		(this->*m_callbacks[m_state]) (Data[counter]);
	}
	return m_state != OnParseError;
}

std::string HttpParser::header (const std::string& HeaderName)
{
	return m_headers[HeaderName];
}

std::string HttpParser::cookie (const std::string& CookieName)
{
	return m_cookies[CookieName];
}

// private

void HttpParser::parseMethod (char ch)
{
	if (ch == ' ') {
		m_state = OnUri;
		m_method = ExpressionMethod; // default value.

		for (Method method = Options; method < ExpressionMethod; ++method) {
			if (m_token == m_method_strings[method]) {
				m_method = method;
				m_token.erase();
				std::cout << "Method: " << m_method << std::endl;
				return;
			}	
		}
		m_token.erase();
	} else
		m_token.append(1, ch);
}

void HttpParser::parseUri (char ch)
{
	if (ch == ' ') {
		m_uri = m_token;
		m_state = OnHttpVersion;
		m_token.erase();
		if (!parse(m_uri)) // bad uri.
			m_state = OnParseError;
		else {
			std::cout << "Scheme: " << scheme() << std::endl;
			std::cout << "Authority: " << authority() << std::endl;
			std::cout << "Path: " << path() << std::endl;
			std::cout << "Query: " << query() << std::endl;
			std::cout << "Fragment: " << fragment() << std::endl;
		}
	} else
		m_token.append(1, ch);
}

void HttpParser::parseHttpVersion (char ch)
{
	if (ch == '\n') {
		if (m_token.length() != 8) { // length of 'HTTP/x.x' is 8.
			m_state = OnParseError;
			return;
		}
		m_state = OnHeaderName;

		switch (m_token[5]) {
			case '0':
				m_version.m_major = 0;
			break;
			case '1':
				m_version.m_major = 1;
			break;
			default:
				m_state = OnParseError;
			break;
		}

		switch (m_token[7]) {
			case '0':
				m_version.m_minor = 0;
			break;
			case '1':
				m_version.m_minor = 1;
			break;
			case '9':
				m_version.m_minor = 9;
			break;
			default:
				m_state = OnParseError;
			break;
		}

		if (m_state != OnParseError) {
			std::cout << "HTTP/" << m_version.m_major
				<< "." << m_version.m_minor << std::endl;
		}

		m_token.erase();
	} else if (ch != '\r')
		m_token.append(1, ch);
}

void HttpParser::parseHeaderName (char ch)
{
	if (ch == '\n') {
		std::cout << "Content-Length: " << m_content_length << std::endl;
		m_state = m_content_length ? OnBody : OnComplete;
		m_token.erase();
	} else if (ch == ' ') {
		m_current_header = m_token;
		m_state = OnHeaderValue;
		m_token.erase();
	} else if (ch != ':')
		m_token.append(1, tolower(ch));
}

void HttpParser::parseHeaderValue (char ch)
{
	if (ch == '\n') {
		m_headers[m_current_header] = m_token;
		if (m_current_header == "content-length")
			m_content_length = atoi(m_token.c_str());
		else if (m_current_header == "cookie")
			parseCookies(m_token);
		m_state = OnHeaderName;
		m_token.erase();
	} else if (ch != '\r')
		m_token.append(1, ch);
}

void HttpParser::parseBody (char ch)
{
	m_body.append(1, ch);
	m_token.append(1, ch);
	if (m_token.length() == m_content_length) {
		m_state = OnComplete;
		if (m_method == Post)
			parseQuery(m_body);
	}
}

void HttpParser::parseCookies (const std::string& CookieStr)
{
	if (CookieStr.empty())
		return;
	std::string::const_iterator start = CookieStr.begin();
	std::string::const_iterator end = CookieStr.end();
	boost::match_results<std::string::const_iterator> what;
	boost::match_flag_type flags = boost::match_default;
	while (boost::regex_search(start, end, what, m_cookie_regex, flags))
	{
		std::string cookie_name (what[1].first, what[1].second);
		std::string cookie_value (what[2].first, what[2].second);
		std::cout << "Cookie Name: " << cookie_name << std::endl;
		std::cout << "Cookie Value: " << cookie_value << std::endl;
		m_cookies[cookie_name] = cookie_value;
		start = what[0].second;
	}
}

} // namespace koohar
