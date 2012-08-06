#ifndef koohar_uri_parser_hh
#define koohar_uri_parser_hh

#include <string>
#include <map>

#include <regex>

namespace koohar {

class UriParser
{
public:
	typedef std::map<std::string, std::string> StringMap;

public:
	UriParser();
	bool parse (const std::string& uri);
	std::string scheme () const { return m_scheme; }
	std::string authority () const { return m_authority; }
	std::string path () const { return m_path; }
	std::string query () const { return m_query; }
	std::string fragment () const { return m_fragment; }

	std::string cookie (const std::string& CookieName);
	std::string body (const std::string& QueryName);

protected:
	void parseQuery (const std::string& QueryString);

protected:
	StringMap m_queries;

private:
	char fromHex (const char ch);
	std::string decode (const std::string& Uri);

private:
	static std::regex m_uri_regex;
	static std::regex m_query_regex;
	std::string m_scheme;
	std::string m_authority;
	std::string m_path;
	std::string m_query;
	std::string m_fragment;

}; // class UriParser

} // namespace koohar

#endif // koohar_uri_parser_hh
