#include "parser.hh"
#include "file.hh"
#include "filemapping.hh"

#ifdef _DEBUG
#include <iostream>
#endif /* _DEBUG */

namespace koohar {

namespace xml {

bool Parser::parse(const std::string& FileName)
{
	if (FileName.empty())
		return false;
	File xml_file(FileName);
	if (!xml_file.open(File::ReadOnly)) {
#ifdef _DEBUG
		std::cout << "[Parser::parse] error opening file " << FileName << std::endl;
#endif /* _DEBUG */
		return false;
	}
	m_size = xml_file.size();
	m_offset = 0;
	FileMapping xml_map(xml_file.fh());
	m_mapped_file = xml_map.map(m_size, 0);
	m_xml.clear();
	while (readObject(m_xml));
	parseIntro();
	return true;
}

// private methods

bool Parser::readObject(Object& Obj)
{
	std::string text;
	readText(text);
	if (!text.empty()) // add as text property
		Obj.property(Property("text", text));
	std::string tag;
	bool tag_closed;
	bool object_closed;
	while (readTag(tag, tag_closed, object_closed)) { // it wasn't closing tag, so still try to parse object
		if (tag.empty())
			return false; // some kind of error actualy
		Object NewObj(tag);
		if (object_closed) { // ' <object/> '
			Obj.object(NewObj);
			return true;
		}
		if (!tag_closed) {
			std::string name;
			std::string value;
			while (readProperty(name, value, object_closed)) {
				if (!name.empty() && !value.empty()) // add proprty only if there is something to add
					NewObj.property(Property(name, value));
				if (object_closed) { // ' <object abracadabra /> '
					Obj.object(NewObj);
					return true;
				}
			}
			if (!name.empty() && !value.empty()) // do it one more time
				NewObj.property(Property(name, value));
			if (object_closed) { // ' <object abracadabra />
				Obj.object(NewObj);
				return true;
			}
		}
		// actualy to this moment tag should be closed anyway
		while (readObject(NewObj)) ; // read included objects
		Obj.object(NewObj);
	}
	return false;
}

void Parser::readText(std::string& text)
{
	char ch;
	size_t nesting = 0; // in comments ('<!' elements) could be nested elements with tags, so we need to be sure all nested elements are closed
	while (getCh(ch)) {
		if (ch == '<') {
			if (!nesting) {
				char next_ch;
				if (preCh(next_ch) && next_ch != '!')
					return;
			}
			++nesting;
		} else if (ch == '>')
			--nesting;
		text.append(1, ch);
	}
}

bool Parser::readTag(std::string& tag, bool& tag_closed, bool& object_closed)
{
	tag = "";
	tag_closed = false;
	object_closed = false;
	char ch;
	while (getCh(ch)) {
		if (ch == '/') {
			if (tag.empty()) {	// "</tag>"
				while (getCh(ch) && ch != '<'); // seek to next '<'
				return true;
			}
			else				// "<tag/>"
				object_closed = true;
		} else if (ch == '>') {
			tag_closed = true;
			return true;
		} else if (ch == ' ')
			return true;
		else 
			tag.append(1, ch);
	}
	return false; // never should get here. parse error ( or end of file ? )
}

bool Parser::readProperty(std::string& name, std::string& value, bool& object_closed)
{
	name = "";
	value = "";
	object_closed = false;
	char ch;
	bool read_value = false;
	size_t quote_count = 0;
	while (getCh(ch)) {
		if (ch == '=' && (quote_count == 0))
			read_value = true;
		else if (ch == ' ' && quote_count == 2)
			return true;
		else if ((ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r') && !quote_count)
			continue; // skip empty fields
		else if (ch == '>')
			return false;
		else if (ch == '/' && (quote_count == 0 || quote_count == 2))
			object_closed = true;
		else {
			if (read_value) {
				if (ch == '"')
					++quote_count;
				else if (quote_count == 1)
					value.append(1, ch);
			}
			else
				name.append(1, ch);
		}
	}
	return false; // never should get here. parse error
}

bool Parser::getCh(char& Ch)
{
	if (m_offset <= m_size) {
		Ch = m_mapped_file[m_offset++];
		return true;
	}
	return false;
}

bool Parser::preCh(char& Ch)
{
	if (m_offset < m_size) {
		Ch = m_mapped_file[m_offset];
		return true;
	}
	return false;
}

Parser::PropPredicate::PropPredicate(const std::string& NewPropName) : m_prop_name(NewPropName) {}

bool Parser::PropPredicate::operator() (Property& Prop) const
{
	return !Prop.name().compare(m_prop_name);
}

Parser::ObjPredicate::ObjPredicate(const std::string& NewKooharTag,
	const std::string& NewPropName) :
	m_tag(NewKooharTag), m_prop_name(NewPropName)
{}

void Parser::ObjPredicate::operator() (Object& Obj) const
{
	const Property* src_prop = Obj.property(PropPredicate(m_prop_name));
	if (!src_prop)
		return;
	if (!Obj.tag().compare(m_tag) && !src_prop->value().empty()) {
		Parser m_doc_dom(m_tag, m_prop_name);
		if (m_doc_dom.parse(src_prop->value()))
			m_doc_dom.copy(Obj);
		Obj.property(Property(m_prop_name)); // Delete 'src' property: add property 'src' with zero-value
	}
}

void Parser::parseIntro()
{
	m_xml.objects(ObjPredicate(m_koohar_tag, m_koohar_path_prop_name));
}

}; // namespace xml

}; // namespace koohar
