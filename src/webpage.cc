#include "webpage.hh"

#include <algorithm>
#include <sstream>

#ifdef _DEBUG
#include <iostream>
#endif /* _DEBUG */

namespace koohar {

WebPage::WebPage (Request& Req, Response& Res, const std::string& FileName) :
	m_req(Req), m_res(Res), m_file_name(FileName), m_sent(false)
{
	m_dom.parse(m_file_name); // To know everything about page we need parse it
}

WebPage::~WebPage()
{
}

void WebPage::render ()
{
	if (m_sent)
		return;
	for (iterator it = begin(); it != end(); ++it) { // Apply all changes to parsed dom
		m_dom.objects(ObjectPredicate<FindByRule, ApplyWebuiObject>(
			FindByRule(it->first), ApplyWebuiObject(it->second)
		));
	}
	std::stringstream parsed_page;
	m_dom.printIntro(parsed_page);
	m_res.writeHead(200);
	m_res.header("Content-Type", "text/html; charset=UTF-8"); // don't admit any other charset ;)
	const std::string& send_string = parsed_page.str();
	m_res.end(send_string.c_str(), send_string.length());
}

CastWebuiObject::CastWebuiObject(xml::Object& Obj, webui::ObjectPtr WObj) : m_obj(&Obj), m_wobj(WObj) {}

void CastWebuiObject::operator() (webui::ObjectPtr Obj)
{
	if (Obj == m_wobj) // Do not cast self
		return;
	xml::Object child(Obj->tag()); // Create new xml::Object with known tag
	for (auto it = Obj->begin(); it != Obj->end(); ++it) // Add all properties
		child.property(xml::Property(it->first, it->second));
	Obj->everyChild(CastWebuiObject(child, Obj)); // Also cast all his children
	m_obj->object(child); // And append new child to Object
}

ApplyWebuiObject::ApplyWebuiObject(webui::ObjectPtr Obj) : m_obj(Obj) {}

void ApplyWebuiObject::operator()(xml::Object& Obj) const
{
	if (!m_obj->tag().empty()) // user did not specified new tag
		Obj.tag(m_obj->tag());
	for (auto it = m_obj->begin(); it != m_obj->end(); ++it)
		Obj.property(xml::Property(it->first, it->second));
	m_obj->everyChild(CastWebuiObject(Obj, m_obj));
}

FindByRule::FindByRule (const std::string& SearchRule) : m_rule(SearchRule),
	m_search_by_id(false), m_search_by_class(false)
{
	m_search_by_id = m_rule[0] == '#';
	m_search_by_class = m_rule[0] == '.';
	if (m_search_by_id || m_search_by_class)
		m_rule.erase(0, 1);
}

bool FindByRule::operator() (xml::Property& Prop) const {
	if (m_search_by_id)
		return (!Prop.name().compare("id") && !Prop.value().compare(m_rule));
	else if (m_search_by_class)
		return (!Prop.name().compare("class") && !Prop.value().compare(m_rule));
	return false;
}

}; // namespace koohar

