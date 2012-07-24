#ifndef koohar_http_parser_hh
#define koohar_http_parser_hh

#include <boost/regex.hpp>
#include <string>

#include "uri_parser.hh"

namespace koohar {

/**
 * This is FSM based http parser.
 */
class HttpParser : public UriParser {
public:

	typedef struct {
		unsigned short int m_major;
		unsigned short int m_minor;
	} Version;

	typedef enum {
		Options,
		Get,
		Head,
		Post,
		Put,
		Delete,
		Trace,
		Connect,
		ExpressionMethod
	} Method;

	typedef enum {
		BadMethod,
		BadUri,
		BadHttpVersion,
		BadHeaders,
		BadBody
	} Error;

public:
	HttpParser();

	/**
	 * Makes parser to eat more data and parse it. Could be called multiple
	 * times.
	 * \param Data Pointer to data that should be parsed.
	 * \param Size Size of data pointed ealrier.
	 * \return false on parse error, true otherwise.
	 */
	bool update (const char* Data, unsigned int Size);
	bool isBad () const { return m_state == OnParseError; }
	bool isComplete () const { return m_state == OnComplete; }
	Method method () const { return m_method; }
	std::string uri () const { return m_uri; }
	Version version () const { return m_version; }
	std::string header (const std::string& HeaderName);
	std::string cookie (const std::string& CookieName);

protected:
	Method m_method;
	std::string m_uri;
	Version m_version;
	StringMap m_headers;
	std::string m_body;
	StringMap m_cookies;

private:

	enum { MaxTokenSize = 4096 };

	typedef enum {
		OnMethod,
		OnUri,
		OnHttpVersion,
		OnHeaderName,
		OnHeaderValue,
		OnBody,
		OnComplete,
		OnParseError
	} State;

	typedef void (HttpParser::*StateCallback) (char ch);

	static StateCallback m_callbacks[OnParseError];
	static const char* m_method_strings[ExpressionMethod];
	static boost::regex m_cookie_regex;

	void parseMethod (char ch);
	void parseUri (char ch);
	void parseHttpVersion (char ch);
	void parseHeaderName (char ch);
	void parseHeaderValue (char ch);
	void parseBody (char ch);

	void parseCookies (const std::string& CookieStr);

private:
	State m_state;
	std::string m_token;
	std::string m_current_header;

	unsigned long long m_content_length;

}; // class HttpParser

} // namespace koohar

#endif // koohar_http_parser_hh
