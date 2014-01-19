#ifndef koohar_json_hh
#define koohar_json_hh

#include <map>
#include <vector>
#include <string>
#include <cstddef>
#include <sstream>
#include <type_traits>

namespace koohar {

namespace JSON {

enum class Type {
  Undefined,
  Boolean,
  Integer,
  Float,
  String,
  Array,
  Collection
};

class Object {
public:

  Object ()
      : m_state(OnValue),
      m_type(Type::Undefined),
      m_boolean(false),
      m_integer(0L),
      m_float(.0)
  {}

  explicit Object(const bool Bool)
      : m_state(OnSuccess),
      m_type(Type::Boolean),
      m_boolean(Bool),
      m_integer(0L),
      m_float(.0)
  {}

  explicit Object(const long Int)
      : m_state(OnSuccess),
      m_type(Type::Integer),
      m_boolean(false),
      m_integer(Int),
      m_float(.0)
  {}

  explicit Object(const double Floating)
      : m_state(OnSuccess),
      m_type(Type::Float),
      m_boolean(false),
      m_integer(0L),
      m_float(Floating)
  {}

  explicit Object(const std::string& Str)
      : m_state(OnSuccess),
      m_type(Type::String),
      m_boolean(false),
      m_integer(0L),
      m_float(.0),
      m_string(Str)
  {}

  Type type() const { return m_type; }
  bool hasType(const Type SomeType) const { return SomeType == m_type; }

  bool& getBoolean() { return m_boolean; }
  bool getBoolean() const { return m_boolean; }
  bool setBoolean (const bool Bool);

  long& getInteger() { return m_integer; }
  long getInteger() const { return m_integer; }
  bool setInteger (const long Int);
  template <typename Trivial>
  bool setTrivial(Trivial Value, std::true_type, std::false_type);

  double& getFloat() { return m_float; }
  double getFloat() const { return m_float; }
  bool setFloat (const double Floating);
  template <typename Trivial>
  bool setTrivial(Trivial Value, std::false_type, std::true_type) {
    return setFloat(static_cast<double>(Value));
  }

  std::string& getString() { return m_string; }
  std::string getString() const { return m_string; }
  bool setString (const std::string& Str);
  template <typename Trivial>
  bool setTrivial(Trivial Value, std::false_type, std::false_type) {
    return setString(std::string(Value));
  }

  void clear();
  template <typename Trivial>
  Object& operator=(const Trivial Value);

  std::vector<Object>& getArray() { return m_array; }
  std::vector<Object> getArray() const { return m_array; }
  bool setArray (const std::vector<Object>& Objects);
  bool addToArray (const Object& Obj);
  bool remove (const std::size_t Index);
  Object& operator[](const std::size_t Index);

  std::map<std::string, Object>& getCollection() { return m_collection; }
  std::map<std::string, Object> getcollection() const {
    return m_collection;
  }
  bool setCollection (const std::map<std::string, Object> ObjCollection);
  bool addToCollection (const std::string& Name, const Object& Obj);
  bool remove (const std::string& Name);
  Object& operator[](const std::string& Name);

  bool empty () const {
    return m_state == OnValue && m_type == Type::Undefined;
  }
  std::string toString() const;
  std::size_t parse(const std::string &Stream);
  bool errorParsing () const { return m_state != OnSuccess; }

private:
  template <typename Type>
  class is_boolean {
  public:
    static const bool value = false;
  }; // class is_boolean<Type>

  enum State {
    OnValue,
    OnArrayObject,
    OnComma,
    OnName,
    OnColon,
    OnCollectionObject,
    OnSuccess,
    OnError
  }; // enum State

private:
  void checkUndefined (const Type SetType) {
    if (m_type == Type::Undefined)
      m_type = SetType;
  }
  template <typename Trivial>
  std::string trivialToString(const Trivial& Value) const {
    std::stringstream stream;
    stream << Value;
    return stream.str();
  }
  std::string stringToString() const;
  std::string arrayToString() const;
  std::string collectionToString() const;
  static bool isSeporator (const char Ch);
  static bool isBoolean (const std::string& Token);
  static bool isInteger (const std::string& Token);
  static bool isFloat (const std::string& Token);
  static bool isString (const std::string& Token);

  void parseValue (const std::string& Stream, std::size_t& Parsed);
  void parseArrayObject (const std::string& Stream, std::size_t& Parsed);
  void parseComma (const std::string& Stream, std::size_t& Parsed);
  void parseName (const std::string& Stream, std::size_t& Parsed);
  void parseColon (const std::string& Stream, std::size_t& Parsed);
  void parseCollectionObject (const std::string& Stream,
                              std::size_t& Parsed);

private:
  State m_state;
  std::string m_token;
  Type m_type;
  bool m_boolean;
  long m_integer;
  double m_float;
  std::string m_string;
  std::vector<Object> m_array;
  std::map<std::string, Object> m_collection;
}; // class Object

template <>
class Object::is_boolean<bool> {
public:
  static const bool value = true;
}; // class is_boolean<bool>

template <typename Trivial>
bool Object::setTrivial(Trivial Value, std::true_type, std::false_type) {
  if (is_boolean<Trivial>::value)
    return setBoolean(Value);
  return setInteger(static_cast<long>(Value));
}

template <typename Trivial>
Object& Object::operator = (const Trivial Value)
{
   setTrivial(Value, typename std::is_integral<Trivial>::type(),
     typename std::is_floating_point<Trivial>::type());
   return *this;
}

std::string strigify(const Object& Obj) { return Obj.toString(); }
Object parse(const std::string& Stream);

} // namespace JSON

} // namespace koohar

#endif // koohar_json_hh
