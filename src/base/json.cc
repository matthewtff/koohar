#include "base/json.hh"

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "base/utils.hh"

namespace koohar {

namespace {

bool IsSeporator(const char ch) {
  static const char kSeporators[] = {',', ':', '{', '}', '[', ']'};
  static const char* kSeporatorsEnd = kSeporators + array_size(kSeporators); 
  if (std::isspace(ch)) {
    return true;
  }
  return std::find(kSeporators, kSeporatorsEnd, ch) != kSeporatorsEnd;
}

bool IsBoolean(const std::string &token) {
  return (token == "true") || (token == "false");
}

bool IsInteger(const std::string &token) {
  for (std::size_t index = 0; index < token.size(); ++index) {
    if (index == 0 && token[0] == '-') {
      continue;
    }
    if (!std::isdigit(token[index])) {
      return false;
    }
  }
  return !token.empty();
}

bool IsFloat(const std::string& token) {
  bool was_dot = false;
  for (std::size_t index = 0; index < token.size(); ++index) {
    const char ch = token[index];
    if (index == 0 && ch == '-') {
      continue;
    }
    if (!std::isdigit(ch)) {
      if (ch != '.' || was_dot) {
        return false;
      }
      was_dot = true;
    }
  }
  return !token.empty();
}

bool IsString(const std::string& token) {
  // Even an empty string is at least 2 symbols legnth: '""'.
  if (token.length() < 2) {
    return false;
  }
  return token[0] == '"' && token[token.length() - 1] == '"';
}

bool IsOpenedString(const std::string& token) {
  if (token.empty() || token[0] != '"') {
    return false;
  }
  if (token.length() > 1) {
    return token[token.length() - 1] != '"';
  }
  return true;
}

}  // anonymous namespace

namespace JSON {

void Object::SetType(const Type type) {
  if (Empty()) {
    type_ = type;
  }
}

bool Object::SetBoolean(const bool boolean) {
  SetTypeIfUndefined(Type::Boolean);
  if (!HasType(Type::Boolean)) {
    return false;
  }
  boolean_ = boolean;
  return true;
}

bool Object::SetInteger(const long integer) {
  SetTypeIfUndefined(Type::Integer);
  if (!HasType(Type::Integer)) {
    return false;
  }
  integer_ = integer;
  return true;
}

bool Object::SetFloat(const double floating) {
  SetTypeIfUndefined(Type::Float);
  if (!HasType(Type::Float)) {
    return false;
  }
  float_ = floating;
  return true;
}

bool Object::SetString(const std::string& str) {
  SetTypeIfUndefined(Type::String);
  if (!HasType(Type::String)) {
    return false;
  }
  string_ = str;
  return true;
}

void Object::Clear() {
  state_ = OnValue;
  type_ = Type::Undefined;
  boolean_ = false;
  integer_ = 0L;
  float_ = .0;
  string_.clear();
  array_.clear();
  collection_.clear();
}

bool Object::SetArray(const std::vector<Object>& objects) {
  SetTypeIfUndefined(Type::Array);
  if (!HasType(Type::Array)) {
    return false;
  }
  array_ = objects;
  return true;
}

bool Object::AddToArray(const Object &obj) {
  SetTypeIfUndefined(Type::Array);
  if (!HasType(Type::Array)) {
    return false;
  }
  array_.push_back(obj);
  return true;
}

bool Object::Remove(const std::size_t index) {
  if (!HasType(Type::Array)) {
    return false;
  }
  if (index >= array_.size()) {
    return false;
  }
  array_.erase(array_.begin() + index);
  return false;
}

Object& Object::operator [](const std::size_t index) {
  if ((!HasType(Type::Array)) || (index >= array_.size())) {
    return *this;
  }
  return array_[index];
}

bool Object::SetCollection(
    const std::map<std::string, Object>& objects_collection) {
  SetTypeIfUndefined(Type::Collection);
  if (!HasType(Type::Collection)) {
    return false;
  }
  collection_ = objects_collection;
  return true;
}

bool Object::AddToCollection(const std::string& name, const Object& obj) {
  SetTypeIfUndefined(Type::Collection);
  if (!HasType(Type::Collection)) {
    return false;
  }
  collection_[name] = obj;
  return true;
}

bool Object::Remove(const std::string &name) {
  if (!HasType(Type::Collection)) {
    return false;
  }
  collection_.erase(name);
  return true;
}

Object& Object::operator [](const std::string& name) {
  SetTypeIfUndefined(Type::Collection);
  if (!HasType(Type::Collection)) {
    return *this;
  }
  return collection_[name];
}

std::string Object::ToString() const {
  switch (type()) {
    case Type::Undefined: return "undefined";
    case Type::Boolean: return boolean_ ? "true" : "false";
    case Type::Integer: return TrivialToString(integer_);
    case Type::Float: return TrivialToString(float_);
    case Type::String: return StringToString();
    case Type::Array: return ArrayToString();
    case Type::Collection: return CollectionToString();
    default: NOTREACHED(); return std::string();
  }
}

std::size_t Object::Parse(const std::string& stream) {
  if (!Empty()) {
    return 0;
  }
  std::size_t parsed = 0;
  while (state_ != OnSuccess && state_ != OnError) {
    switch (state_) {
      case OnValue:
        ParseValue(stream, parsed);
      break;
      case OnArrayObject:
        ParseArrayObject(stream, parsed);
      break;
      case OnComma:
        ParseComma(stream, parsed);
      break;
      case OnName:
        ParseName(stream, parsed);
      break;
      case OnColon:
        ParseColon(stream, parsed);
      break;
      case OnCollectionObject:
        ParseCollectionObject(stream, parsed);
      break;
      default:
        state_ = OnError;
    }
  }
  return parsed;
}

std::string Object::StringToString() const {
  std::stringstream stream;
  stream << "\"";
  std::for_each(string_.begin(), string_.end(), [&stream](const char ch) {
    if (ch == '"') {
      stream << '\\' << '"';
    } else {
      stream << ch;
    }
  });
  stream << "\"";
  return stream.str();
}

std::string Object::ArrayToString() const {
  std::stringstream stream;
  stream << "[";
  for (size_t index = 0; index < array_.size(); ++index) {
    if (0 != index)
      stream << ",";
    stream << array_[index].ToString();
  }
  stream << "]";
  return stream.str();
}

std::string Object::CollectionToString() const {
  std::stringstream stream;
  stream << "{";
  for (auto obj = collection_.begin(); obj != collection_.end(); ++obj) {
    if (obj != collection_.begin())
      stream << ",";
    stream << "\"" << obj->first << "\":" << obj->second.ToString();
  }
  stream << "}";
  return stream.str();
}

void Object::ParseValue(const std::string& stream, std::size_t& parsed) {
  const char ch = stream[parsed];
  if (IsOpenedString(token_) || !IsSeporator(ch)) {
    if (ch == '"' && !token_.empty() && token_[token_.length() - 1] == '\\') {
      token_.erase(token_.end() - 1);
    }
    token_.append(1, ch);
    ++parsed;
    return;
  }
  if (std::isspace(ch) && token_.empty()) {
    ++parsed;
    return;
  }
  if (::koohar::IsBoolean(token_)) {
    SetBoolean(token_ == "true");
    state_ = OnSuccess;
  } else if (IsInteger(token_)) {
    long value;
    std::istringstream(token_) >> value;
    SetInteger(value);
    state_ = OnSuccess;
  } else if (IsFloat(token_)) {
    double value;
    std::istringstream(token_) >> value;
    SetFloat(value);
    state_ = OnSuccess;
  } else if (IsString(token_)) {
    token_.erase(0, 1);
    token_.erase(token_.end() - 1);
    SetString(token_);
    state_ = OnSuccess;
  } else if ('[' == ch) {
    state_ = OnArrayObject;
    token_.clear();
    ++parsed;
  } else if ('{' == ch) {
    state_ = OnName;
    token_.clear();
    ++parsed;
  } else {
    state_ = OnError;
  }
}

void Object::ParseArrayObject(const std::string& stream, std::size_t& parsed) {
  Object object;
  std::string remaining(stream);
  remaining.erase(0, parsed);
  parsed += object.Parse(remaining);
  if (object.ErrorParsing()) {
    state_ = OnError;
    return;
  }
  AddToArray(object);
  state_ = OnComma;
  token_.clear();
}

void Object::ParseComma(const std::string& stream, std::size_t& parsed) {
  const char ch = stream[parsed];
  ++parsed;
  if (std::isspace(ch)) {
    return;
  }
  if (ch == ',') {
    state_ = (type_ == Type::Array) ? OnArrayObject : OnName;
  } else if ((ch == ']' && type_ == Type::Array) ||
    (ch == '}' && type_ == Type::Collection)) {
    state_ = OnSuccess;
  } else {
    state_ = OnError;
  }
}

void Object::ParseName(const std::string& stream, std::size_t& parsed) {
  const char ch = stream[parsed];
  ++parsed;
  if (token_.empty()) {
    if (std::isspace(ch)) {
      return;
    }
    if (ch != '"') {
      state_ = OnError;
      return;
    }
  } else if (ch == '"') {
    token_.erase(0, 1);
    state_ = OnColon;
    return;
  }
  token_.append(1, ch);
}

void Object::ParseColon(const std::string& stream, std::size_t& parsed) {
  const char ch = stream[parsed];
  ++parsed;
  if (std::isspace(ch)) {
    return;
  }
  state_ = (ch == ':') ? OnCollectionObject : OnError;
}

void Object::ParseCollectionObject(const std::string& stream,
                                   std::size_t& parsed) {
  Object object;
  std::string remaining(stream);
  remaining.erase(0, parsed);
  parsed += object.Parse(remaining);
  if (object.ErrorParsing()) {
    state_ = OnError;
    return;
  }
  AddToCollection(token_, object);
  state_ = OnComma;
  token_.clear();
}

std::string Stringify(const Object& object) { return object.ToString(); }

Object Parse(const std::string& stream) {
  Object result;
  result.Parse(stream);
  return result;
}

} // namespace JSON

} // namespace koohar
