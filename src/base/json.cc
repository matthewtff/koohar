#include "base/json.hh"

#include <map>
#include <vector>
#include <string>
#include <cstddef>
#include <sstream>
#include <cctype>

#include "base/utils.hh"

namespace koohar {

namespace JSON {

bool Object::setBoolean(const bool Bool) {
  checkUndefined(Type::Boolean);
  if (!hasType(Type::Boolean))
    return false;
  m_boolean = Bool;
  return true;
}

bool Object::setInteger(const long Int) {
  checkUndefined(Type::Integer);
  if (!hasType(Type::Integer))
    return false;
  m_integer = Int;
  return true;
}

bool Object::setFloat(const double Floating) {
  checkUndefined(Type::Float);
  if (!hasType(Type::Float))
    return false;
  m_float = Floating;
  return true;
}

bool Object::setString(const std::string& Str) {
  checkUndefined(Type::String);
  if (!hasType(Type::String))
    return false;
  m_string = Str;
  return true;
}

void Object::clear() {
  m_state = OnValue;
  m_type = Type::Undefined;
  m_boolean = false;
  m_integer = 0L;
  m_float = .0;
  m_string.clear();
  m_array.clear();
  m_collection.clear();
}

bool Object::setArray(const std::vector<Object>& Objects) {
  checkUndefined(Type::Array);
  if (!hasType(Type::Array))
    return false;
  m_array = Objects;
  return true;
}

bool Object::addToArray(const Object &Obj) {
  checkUndefined(Type::Array);
  if (!hasType(Type::Array))
    return false;
  m_array.push_back(Obj);
  return true;
}

bool Object::remove(const std::size_t Index) {
  if (!hasType(Type::Array))
    return false;
  if (Index >= m_array.size())
    return false;
  m_array.erase(m_array.begin() + Index);
  return false;
}

Object& Object::operator [](const std::size_t Index) {
  if (!hasType(Type::Array))
    return *this;
  if (Index >= m_array.size())
    return *this;
  return m_array[Index];
}

bool Object::setCollection(const std::map<std::string, Object>& ObjCollection) {
  checkUndefined(Type::Collection);
  if (!hasType(Type::Collection))
    return false;
  m_collection = ObjCollection;
  return true;
}

bool Object::addToCollection(const std::string& Name, const Object& Obj) {
  checkUndefined(Type::Collection);
  if (!hasType(Type::Collection))
    return false;
  m_collection[Name] = Obj;
  return true;
}

bool Object::remove(const std::string &Name) {
  if (!hasType(Type::Collection))
    return false;
  m_collection.erase(Name);
  return true;
}

Object& Object::operator [](const std::string& Name) {
  checkUndefined(Type::Collection);
  if (!hasType(Type::Collection))
    return *this;
  return m_collection[Name];
}

std::string Object::toString() const {
  switch (type()) {
    case Type::Undefined: return "undefined";
    case Type::Boolean: return m_boolean ? "true" : "false";
    case Type::Integer: return trivialToString(m_integer);
    case Type::Float: return trivialToString(m_float);
    case Type::String: return stringToString();
    case Type::Array: return arrayToString();
    case Type::Collection: return collectionToString();
    default: NOTREACHED();
  }
}

std::size_t Object::parse(const std::string& Stream) {
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

std::string Object::stringToString() const {
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

std::string Object::arrayToString() const {
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

std::string Object::collectionToString() const {
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

bool Object::isSeporator(const char Ch) {
  return std::isspace(Ch) || Ch == ',' || Ch == ':' ||
    Ch == '{' || Ch == '}' || Ch == '[' || Ch == ']';
}

bool Object::isBoolean(const std::string &Token) {
  return (Token == "true") || (Token == "false");
}

bool Object::isInteger(const std::string &Token) {
  for (std::size_t index = 0; index < Token.size(); ++index) {
    if (index == 0 && Token[0] == '-')
      continue;
    if (!std::isdigit(Token[index]))
      return false;
  }
  return !Token.empty();
}

bool Object::isFloat(const std::string& Token) {
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

bool Object::isString(const std::string& Token) {
  if (Token.empty())
    return false;
  return Token[0] == '"' && Token[Token.length() - 1] == '"';
}

void Object::parseValue(const std::string& Stream, std::size_t& Parsed) {
  const char ch = Stream[Parsed];
  if (!isSeporator(ch) || (!m_token.empty() &&
    m_token[0] == '"' && m_token[m_token.length() - 1] != '"')) {
    if (ch == '"' && !m_token.empty() &&
        m_token[m_token.length() - 1] == '\\') {
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

void Object::parseArrayObject(const std::string& Stream, std::size_t& Parsed) {
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

void Object::parseComma(const std::string& Stream, std::size_t& Parsed) {
  const char ch = Stream[Parsed];
  ++Parsed;
  if (std::isspace(ch))
    return;
  if (ch == ',') {
    m_state = (m_type == Type::Array) ? OnArrayObject : OnName;
  } else if ((ch == ']' && m_type == Type::Array) ||
    (ch == '}' && m_type == Type::Collection)) {
    m_state = OnSuccess;
  } else {
    m_state = OnError;
  }
}

void Object::parseName(const std::string& Stream, std::size_t& Parsed) {
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

void Object::parseColon(const std::string& Stream, std::size_t& Parsed) {
  const char ch = Stream[Parsed];
  ++Parsed;
  if (std::isspace(ch))
    return;
  m_state = (ch == ':') ? OnCollectionObject : OnError;
}

void Object::parseCollectionObject(const std::string& Stream,
                                   std::size_t& Parsed) {
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

std::string strigify(const Object& Obj) { return Obj.toString(); }

Object parse(const std::string& Stream) {
  Object result;
  result.parse(Stream);
  return result;
}

} // namespace JSON

} // namespace koohar
