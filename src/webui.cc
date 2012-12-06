#include "webui.hh"

namespace koohar {

namespace webui {

Object::Object (const std::string& SearchRule, const std::string& SomeTag)
	: m_search_rule(SearchRule), m_tag(SomeTag)
{}

std::string& Object::set (const std::string& Prop)
{
	return this->operator[](Prop);
}

bool Object::appendChild (ObjectPtr Child)
{
	m_children.push_back(Child);
	return true;
}

} // namespace webui

} // namespace koohar
