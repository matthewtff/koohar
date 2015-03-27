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
  Collection,
};

class Object {
 public:
  Object () : state_(OnValue),
              type_(Type::Undefined),
              boolean_(false),
              integer_(0L),
              float_(.0f) {
  }

  explicit Object(const bool Bool) : state_(OnSuccess),
                                     type_(Type::Boolean),
                                     boolean_(Bool),
                                     integer_(0L),
                                     float_(.0f) {
  }

  explicit Object(const long Int) : state_(OnSuccess),
                                    type_(Type::Integer),
                                    boolean_(false),
                                    integer_(Int),
                                    float_(.0f) {
  }

  explicit Object(const double Floating) : state_(OnSuccess),
                                           type_(Type::Float),
                                           boolean_(false),
                                           integer_(0L),
                                           float_(Floating) {
  }

  explicit Object(const std::string& Str) : state_(OnSuccess),
                                            type_(Type::String),
                                            boolean_(false),
                                            integer_(0L),
                                            float_(.0f),
                                            string_(Str) {
  }

  Type type() const { return type_; }
  bool HasType(const Type SomeType) const { return SomeType == type_; }

  bool& GetBoolean() { return boolean_; }
  bool GetBoolean() const { return boolean_; }
  bool SetBoolean(const bool Bool);

  long& GetInteger() { return integer_; }
  long GetInteger() const { return integer_; }
  bool SetInteger(const long Int);
  template <typename Trivial>
  bool SetTrivial(Trivial Value, std::true_type, std::false_type);

  double& GetFloat() { return float_; }
  double GetFloat() const { return float_; }
  bool SetFloat(const double Floating);
  template <typename Trivial>
  bool SetTrivial(Trivial Value, std::false_type, std::true_type) {
    return SetFloat(static_cast<double>(Value));
  }

  std::string& GetString() { return string_; }
  const std::string& GetString() const { return string_; }
  bool SetString(const std::string& Str);
  template <typename Trivial>
  bool SetTrivial(Trivial Value, std::false_type, std::false_type) {
    return SetString(std::string(Value));
  }

  void Clear();
  template <typename Trivial>
  Object& operator=(const Trivial Value);

  std::vector<Object>& GetArray() { return array_; }
  const std::vector<Object>& GetArray() const { return array_; }
  bool SetArray(const std::vector<Object>& Objects);
  bool AddToArray(const Object& Obj);
  bool Remove(const std::size_t Index);
  Object& operator[](const std::size_t Index);

  std::map<std::string, Object>& GetCollection() { return collection_; }
  const std::map<std::string, Object>& Getcollection() const {
    return collection_;
  }
  bool SetCollection(const std::map<std::string, Object>& ObjCollection);
  bool AddToCollection(const std::string& Name, const Object& Obj);
  bool Remove(const std::string& Name);
  Object& operator[](const std::string& Name);

  bool Empty() const {
    return state_ == OnValue && type_ == Type::Undefined;
  }
  std::string ToString() const;
  std::size_t Parse(const std::string &Stream);
  bool ErrorParsing() const { return state_ != OnSuccess; }

 private:
  template <typename Type>
  class IsBoolean {
  public:
    static const bool value = false;
  }; // class IsBoolean<Type>

  enum State {
    OnValue,
    OnArrayObject,
    OnComma,
    OnName,
    OnColon,
    OnCollectionObject,
    OnSuccess,
    OnError,
  }; // enum State

  void SetTypeIfUndefined(const Type type) {
    if (type_ == Type::Undefined) {
      type_ = type;
    }
  }
  template <typename Trivial>
  std::string TrivialToString(const Trivial& value) const {
    std::stringstream stream;
    stream << value;
    return stream.str();
  }
  std::string StringToString() const;
  std::string ArrayToString() const;
  std::string CollectionToString() const;

  void ParseValue(const std::string& stream, std::size_t& parsed);
  void ParseArrayObject(const std::string& stream, std::size_t& parsed);
  void ParseComma(const std::string& stream, std::size_t& parsed);
  void ParseName(const std::string& stream, std::size_t& parsed);
  void ParseColon(const std::string& stream, std::size_t& parsed);
  void ParseCollectionObject(const std::string& stream, std::size_t& parsed);

  State state_;
  std::string token_;
  Type type_;
  bool boolean_;
  long integer_;
  double float_;
  std::string string_;
  std::vector<Object> array_;
  std::map<std::string, Object> collection_;
}; // class Object

template <>
class Object::IsBoolean<bool> {
public:
  static const bool value = true;
}; // class IsBoolean<bool>

template <typename Trivial>
bool Object::SetTrivial(Trivial value, std::true_type, std::false_type) {
  if (IsBoolean<Trivial>::value) {
    return SetBoolean(value);
  }
  return SetInteger(static_cast<long>(value));
}

template <typename Trivial>
Object& Object::operator = (const Trivial value) {
   SetTrivial(value, typename std::is_integral<Trivial>::type(),
     typename std::is_floating_point<Trivial>::type());
   return *this;
}

std::string Stringify(const Object& object);
Object Parse(const std::string& stream);

} // namespace JSON

} // namespace koohar

#endif // koohar_json_hh
