#ifndef koohar_http_parser_hh
#define koohar_http_parser_hh

#include <cstdint>
#include <string>
#include <regex>

#include "uri_parser.hh"

namespace koohar {

/**
 * This is FSM based http parser.
 */
class HttpParser : public UriParser {
public:

	struct Version {
		unsigned short int m_major;
		unsigned short int m_minor;
	};

	enum Method {
		Options,
		Get,
		Head,
		Post,
		Put,
		Delete,
		Trace,
		Connect,
		ExpressionMethod
	};

	enum Error {
		BadMethod,
		BadUri,
		BadHttpVersion,
		BadHeaders,
		BadBody
	};

public:
	HttpParser ();
	virtual ~HttpParser () {}

	/**
	 * Makes parser to eat more data and parse it. Could be called multiple
	 * times.
	 * @param Data Pointer to data that should be parsed.
	 * @param Size Size of data pointed ealrier.
	 * @return false on parse error, true otherwise.
	 */
	bool update (const char* Data, unsigned int Size);
	bool isBad () const { return m_state == OnParseError; }
	bool isComplete () const { return m_state == OnComplete; }
	Method method () const { return m_method; }
	std::string uri () const { return m_uri; }
	Version version () const { return m_version; }
	std::string header (const std::string& HeaderName);
	std::string cookie (const std::string& CookieName);
	std::string body () const;

protected:
	Method m_method;
	std::string m_uri;
	Version m_version;
	StringMap m_headers;
	std::string m_body;
	StringMap m_cookies;

private:

	enum { MaxTokenSize = 4096 };

	enum State {
		OnMethod,
		OnUri,
		OnHttpVersion,
		OnHeaderName,
		OnHeaderValue,
		OnBody,
		OnComplete,
		OnParseError
	};

	typedef void (HttpParser::*StateCallback) (char ch);

	void parseMethod (char ch);
	void parseUri (char ch);
	void parseHttpVersion (char ch);
	void parseHeaderName (char ch);
	void parseHeaderValue (char ch);
	void parseBody (char ch);

	void parseCookies (const std::string& CookieStr);

private:
	static StateCallback m_callbacks[OnParseError];
	static std::string m_method_strings[ExpressionMethod];
	static std::regex m_cookie_regex;

	State m_state;
	std::string m_token;
	std::string m_current_header;

	std::uint64_t m_content_length;

}; // class HttpParser

} // namespace koohar

#endif // koohar_http_parser_hh
