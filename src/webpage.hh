#ifndef koohar_webpage_hh
#define koohar_webpage_hh

#include <map>
#include <string>

#include "request.hh"
#include "response.hh"
#include "webui.hh"
#include "parser.hh"

namespace koohar {

class WebPage : public std::map<std::string, webui::ObjectPtr > {
public:
	WebPage (Request& Req, Response& Res, const std::string& FileName);
	~WebPage ();
	Request& req() { return m_req; }
	Response& res() { return m_res; }
	void render ();
	template<class T> webui::ObjectPtr factory(const std::string& SearchRule,
		const std::string& Tag = "")
	{
		// check if element with that search rule already exists
		iterator it = SearchRule.empty() ? end() : find(SearchRule);
		if (it != end()) { // and if exists
			try {
				return it->second; // try to cast it to T
			} catch (...) { // if impossible
				it->second.reset(); // delete existing element
			}
		}
		webui::ObjectPtr NewObject (new T(SearchRule, Tag)); // and create new one
		this->operator[](SearchRule) = NewObject;
		return NewObject;
	}
	bool sent () const { return m_sent; }
	bool sent (bool sent) { m_sent = sent; return m_sent; }

private:
	Request& m_req;
	Response& m_res;
	xml::Parser m_dom;
	std::string m_file_name;
	bool m_sent; // set true, if you sent data already, and no need to send again.
}; // class WebPage

// These predicates are used to link xml:: and webui:: Objects

// Cast Object from webui:: to xml::
// And not just that object, but also recursivly all his children
class CastWebuiObject {
public:
	CastWebuiObject (xml::Object& Obj, webui::ObjectPtr WObj); // Object to
	void operator() (webui::ObjectPtr Obj);

private:
	xml::Object* m_obj;
	webui::ObjectPtr m_wobj;
}; // class CastObject

// Takes webui::Object and moves all his fields and children to xml::Objects
class ApplyWebuiObject {
public:
	ApplyWebuiObject(webui::ObjectPtr Obj);
	void operator()(xml::Object& Obj) const;

private:
	webui::ObjectPtr m_obj;
}; // class ApplyWebuiObject

// Returns true if current property contains desired id field
class FindByRule {
public:
	FindByRule(const std::string& SearchRule);
	bool operator()(xml::Property& Prop) const;

private:
	std::string m_rule;
	bool m_search_by_id;
	bool m_search_by_class;
}; // class FindIdPredicate

template <class SearchPredicate, class DoPredicate>
class ObjectPredicate {
public:
	ObjectPredicate (const SearchPredicate& SearchPred, const DoPredicate& DoPred)
		: m_search_pred(SearchPred), m_do_pred(DoPred) {}
	void operator() (xml::Object& Obj) const {
		if (Obj.property(m_search_pred))
			m_do_pred(Obj);
	}

private:
	const SearchPredicate& m_search_pred;
	const DoPredicate& m_do_pred;
}; // class ObjectPredicate

}; // namespace koohar

#endif // koohar_webpage_hh
