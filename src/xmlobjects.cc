#include "xmlobjects.hh"

#include <iostream>

namespace koohar {

namespace xml {

void Object::property(const Property& Prop)
{
	m_properties.remove_if(SameName(Prop.name()));
	if (!Prop.value().empty())
		m_properties.push_back(Prop);
}

const std::string Object::text () const
{
	for (auto it = m_properties.begin(); it != m_properties.end(); ++it) {
		if (!it->name().compare("text"))
			return it->value();
	}
	return std::string();
}

void Object::print(std::ostream& out, const std::string& koohar_tag, size_t tab_number)
{
	const bool skip_tag = !m_tag.compare(koohar_tag);
	if (!skip_tag) { // if it is 'koohar' tag, then skip it, just print it's inner objects
		for (size_t count = 0; count < tab_number; count++)
			out << "\t";
		out << "<" << m_tag;
		Property* text_field = 0;
		for (auto it = m_properties.begin(); it != m_properties.end(); ++it)
		{
			if (it->name().compare("text"))
				it->print(out);
			else
				text_field = &(*it);
		}
		out << ">";
		if (text_field)
			out << text_field->value();
	}
	if (!m_objects.empty()) {
		out << "\n";
		for (auto it = m_objects.begin(); it != m_objects.end(); ++it)
			it->print(out, koohar_tag, skip_tag ? tab_number : tab_number + 1);
		if (!skip_tag) // tabs for closing tag not needed if no such
			for (size_t count = 0; count < tab_number; count++)
				out << "\t";
	}
	if (!skip_tag) // no closing tag needed for 'koohar' tag
		out << "</" << m_tag << ">\n";
}

void Object::printIntro(std::ostream& out, const std::string& koohar_tag, size_t tab_number)
{
	for (auto it = m_objects.begin(); it != m_objects.end(); ++it)
		it->print(out, koohar_tag, tab_number + 1);
}

void Object::clear ()
{
	m_properties.clear();
	m_objects.clear();
	m_tag.erase();
}

void Object::copy (Object& CopyTo)
{
	for (auto it = m_objects.begin(); it != m_objects.end(); ++it)
		CopyTo.object(*it);
}

} // namespace xml

} // namespace koohar
