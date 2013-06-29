#include "json.hh"

#include <map>
#include <vector>
#include <string>
#include <cstddef>
#include <sstream>
#include <cctype>

namespace koohar {

namespace JSON {

bool Object::setBoolean(const bool Bool)
{
	checkUndefined(Boolean);
	if (!hasType(Boolean))
		return false;
	m_boolean = Bool;
	return true;
}

bool Object::setInteger(const long Int)
{
	checkUndefined(Integer);
	if (!hasType(Integer))
		return false;
	m_integer = Int;
	return true;
}

bool Object::setFloat(const double Floating)
{
	checkUndefined(Float);
	if (!hasType(Float))
		return false;
	m_float = Floating;
	return true;
}

bool Object::setString(const std::string& Str)
{
	checkUndefined(String);
	if (!hasType(String))
		return false;
	m_string = Str;
	return true;
}

void Object::clear()
{
	m_state = OnValue;
	m_type = Undefined;
	m_boolean = false;
	m_integer = 0L;
	m_float = .0;
	m_string.clear();
	m_array.clear();
	m_collection.clear();
}

bool Object::setArray(const std::vector<Object> Objects)
{
	checkUndefined(Array);
	if (!hasType(Array))
		return false;
	m_array.clear();
	m_array.reserve(Objects.size());
	for (std::size_t index = 0; index < Objects.size(); ++index)
		m_array[index] = Objects[index];
	return true;
}

bool Object::addToArray(const Object &Obj)
{
	checkUndefined(Array);
	if (!hasType(Array))
		return false;
	m_array.push_back(Obj);
	return true;
}

bool Object::remove(const std::size_t Index)
{
	if (!hasType(Array))
		return false;
	if (Index >= m_array.size())
		return false;
	m_array.erase(m_array.begin() + Index);
	return false;
}

Object& Object::operator [](const std::size_t Index)
{
	if (!hasType(Array))
		return *this;
	if (Index >= m_array.size())
		return *this;
	return m_array[Index];
}

bool Object::setCollection(const std::map<std::string, Object> ObjCollection)
{
	checkUndefined(Collection);
	if (!hasType(Collection))
		return false;
	m_collection.clear();
	for (auto obj = ObjCollection.begin(); obj != ObjCollection.end(); ++obj)
		m_collection[obj->first] = obj->second;
	return true;
}

bool Object::addToCollection(const std::string& Name, const Object &Obj)
{
	checkUndefined(Collection);
	if (!hasType(Collection))
		return false;
	m_collection[Name] = Obj;
	return true;
}

bool Object::remove(const std::string &Name)
{
	if (!hasType(Collection))
		return false;
	m_collection.erase(Name);
	return true;
}

Object& Object::operator [](const std::string& Name)
{
	checkUndefined(Collection);
	if (!hasType(Collection))
		return *this;
	return m_collection[Name];
}

std::string Object::toString() const
{
	if (hasType(Undefined))
		return "undefined";
	if (hasType(Boolean))
		return m_boolean ? "true" : "false";
	else if (hasType(Integer))
		return trivialToString(m_integer);
	else if (hasType(Float))
		return trivialToString(m_float);
	else if (hasType(String))
		return stringToString();
	else if (hasType(Array))
		return arrayToString();
	return collectionToString();
}

std::size_t Object::parse(const std::string& Stream)
{
	if (!empty())
		return 0;
	std::size_t parsed = 0;
	while (m_state != OnSuccess && m_state != OnError) {
		switch (m_state) {
			case OnValue:
				parseValue(Stream, parsed);
			break;
			case OnArrayObject:
				parseArrayObject(Stream, parsed);
			break;
			case OnComma:
				parseComma(Stream, parsed);
			break;
			case OnName:
				parseName(Stream, parsed);
			break;
			case OnColon:
				parseColon(Stream, parsed);
			break;
			case OnCollectionObject:
				parseCollectionObject(Stream, parsed);
			break;
			default:
				m_state = OnError;
		}
	}
	return parsed;
}

std::string Object::stringToString() const
{
	std::stringstream stream;
	stream << "\"";
	for (std::size_t index = 0; index < m_string.length(); ++index)
		if (m_string[index] == '"')
			stream << '\\' << '"';
		else
			stream << m_string[index];
	stream << "\"";
	return stream.str();
}

std::string Object::arrayToString() const
{
	std::stringstream stream;
	stream << "[";
	for (size_t index = 0; index < m_array.size(); ++index) {
		if (0 != index)
			stream << ",";
		stream << m_array[index].toString();
	}
	stream << "]";
	return stream.str();
}

std::string Object::collectionToString() const
{
	std::stringstream stream;
	stream << "{";
	for (auto obj = m_collection.begin(); obj != m_collection.end(); ++obj) {
		if (obj != m_collection.begin())
			stream << ",";
		stream << "\"" << obj->first << "\":" << obj->second.toString();
	}
	stream << "}";
	return stream.str();
}

bool Object::isSeporator(const char Ch)
{
	return std::isspace(Ch) || Ch == ',' || Ch == ':' ||
		Ch == '{' || Ch == '}' || Ch == '[' || Ch == ']';
}

bool Object::isBoolean(const std::string &Token)
{
	return (Token == "true") || (Token == "false");
}

bool Object::isInteger(const std::string &Token)
{
	for (std::size_t index = 0; index < Token.size(); ++index) {
		if (index == 0 && Token[0] == '-')
			continue;
		if (!std::isdigit(Token[index]))
			return false;
	}
	return !Token.empty();
}

bool Object::isFloat(const std::string &Token)
{
	bool was_dot = false;
	for (std::size_t index = 0; index < Token.size(); ++index) {
		const char Ch = Token[index];
		if (index == 0 && Ch == '-')
			continue;
		if (!std::isdigit(Ch)) {
			if (Ch != '.' || was_dot)
				return false;
			was_dot = true;
		}
	}
	return !Token.empty();
}

bool Object::isString(const std::string &Token)
{
	if (Token.empty())
		return false;
	return Token[0] == '"' && Token[Token.length() - 1] == '"';
}

void Object::parseValue(const std::string& Stream, std::size_t &Parsed)
{
	const char ch = Stream[Parsed];
	if (!isSeporator(ch) || (!m_token.empty() &&
		m_token[0] == '"' && m_token[m_token.length() - 1] != '"'))
	{
		if (ch == '"' && !m_token.empty() &&
			m_token[m_token.length() - 1] == '\\')
		{
			m_token.erase(m_token.end() - 1);
		}
		m_token.append(1, ch);
		++Parsed;
		return;
	}
	if (std::isspace(ch) && m_token.empty()) {
		++Parsed;
		return;
	}
	if (isBoolean(m_token)) {
		setBoolean(m_token == "true");
		m_state = OnSuccess;
	} else if (isInteger(m_token)) {
		long value;
		std::istringstream(m_token) >> value;
		setInteger(value);
		m_state = OnSuccess;
	} else if (isFloat(m_token)) {
		double value;
		std::istringstream(m_token) >> value;
		setFloat(value);
		m_state = OnSuccess;
	} else if (isString(m_token)) {
		m_token.erase(0, 1);
		m_token.erase(m_token.end() - 1);
		setString(m_token);
		m_state = OnSuccess;
	} else if ('[' == ch) {
		m_state = OnArrayObject;
		m_token.clear();
		++Parsed;
	} else if ('{' == ch) {
		m_state = OnName;
		m_token.clear();
		++Parsed;
	} else
		m_state = OnError;
}

void Object::parseArrayObject(const std::string &Stream, std::size_t &Parsed)
{
	Object obj;
	std::string remaining(Stream);
	remaining.erase(0, Parsed);
	Parsed += obj.parse(remaining);
	if (obj.errorParsing()) {
		m_state = OnError;
		return;
	}
	addToArray(obj);
	m_state = OnComma;
	m_token.clear();
}

void Object::parseComma(const std::string &Stream, std::size_t &Parsed)
{
	const char ch = Stream[Parsed];
	++Parsed;
	if (std::isspace(ch))
		return;
	if (ch == ',') {
		m_state = (m_type == Array) ? OnArrayObject : OnName;
	} else if ((ch == ']' && m_type == Array) ||
		(ch == '}' && m_type == Collection))
	{
		m_state = OnSuccess;
	} else
		m_state = OnError;
}

void Object::parseName(const std::string &Stream, std::size_t &Parsed)
{
	const char ch = Stream[Parsed];
	++Parsed;
	if (m_token.empty()) {
		if (std::isspace(ch))
			return;
		if (ch != '"') {
			m_state = OnError;
			return;
		}
	} else if (ch == '"') {
		m_token.erase(0, 1);
		m_state = OnColon;
		return;
	}
	m_token.append(1, ch);
}

void Object::parseColon(const std::string &Stream, std::size_t &Parsed)
{
	const char ch = Stream[Parsed];
	++Parsed;
	if (std::isspace(ch))
		return;
	if (ch == ':')
		m_state = OnCollectionObject;
	else
		m_state = OnError;
}

void Object::parseCollectionObject(const std::string &Stream,
	std::size_t &Parsed)
{
	Object obj;
	std::string remaining(Stream);
	remaining.erase(0, Parsed);
	Parsed += obj.parse(remaining);
	if (obj.errorParsing()) {
		m_state = OnError;
		return;
	}
	addToCollection(m_token, obj);
	m_state = OnComma;
	m_token.clear();
}

std::string strigify(const Object &Obj)
{
	return Obj.toString();
}

Object parse(const std::string& Stream)
{
	Object result;
	result.parse(Stream);
	return result;
}

} // namespace JSON

} // namespace koohar
