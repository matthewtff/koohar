#ifndef koohar_uri_parser_hh
#define koohar_uri_parser_hh

#include <string>
#include <map>

#include "base/utils.hh"

namespace koohar {

class UriParser {
 public:
	bool parse (const std::string& uri);
	std::string scheme () const { return m_scheme; }
	std::string authority () const { return m_authority; }
	std::string path () const { return m_path; }
	std::string query () const { return m_query; }
	std::string fragment () const { return m_fragment; }

	std::string body (const std::string& QueryName);

 protected:
	void parseQuery (const std::string& QueryString);

 protected:
	StringMap m_queries;

 private:
	std::string m_scheme;
	std::string m_authority;
	std::string m_path;
	std::string m_query;
	std::string m_fragment;
};  // class UriParser

}  // namespace koohar

#endif // koohar_uri_parser_hh
