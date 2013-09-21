#ifndef koohar_parser_hh
#define koohar_parser_hh

#include <string>
#include <sstream>

#include "xmlobjects.hh"

namespace koohar {

namespace xml {

class Parser {
public:
  Parser (const std::string& NewKooharTag = "koohar",
          const std::string& NewKooharPathPropName = "src")
      : m_xml("parser_root"), m_koohar_tag(NewKooharTag),
      m_koohar_path_prop_name(NewKooharPathPropName) {}

  template<class Predicate>
  void objects(const Predicate& Pred) {
    m_xml.objects(Pred);
  }
  bool parse(const std::string& FileName);
  void print(std::ostream& out) {
    m_xml.print(out, m_koohar_tag); out << std::endl;
  }
  void printIntro(std::ostream& out) {
    out << m_xml.text();
    m_xml.printIntro(out, m_koohar_tag);
    out << std::endl;
  }
  void copy(Object& CopyTo) { m_xml.copy(CopyTo); }

private:
  bool readObject(Object& obj);
  void readText(std::string& text);
  bool readTag(std::string& tag, bool& tag_closed, bool& object_closed);
  bool readProperty(std::string& name,
                    std::string& value,
                    bool& object_closed);
  bool getCh(char& Ch);
  bool preCh(char& Ch);
  
  struct PropPredicate {
    PropPredicate(const std::string& NewPropName);
    bool operator() (Property& Prop) const;
  private:
    std::string m_prop_name;
  }; // struct PropPredicate

  struct ObjPredicate {
    ObjPredicate(const std::string& NewKooharTag,
                 const std::string& NewPropName);
    void operator() (Object& Obj) const;
  private:
    std::string m_tag;
    std::string m_prop_name;
  }; // struct ObjPredicate
  
  // Include other files to current
  void parseIntro();

private:
  Object m_xml;
  char* m_mapped_file;
  size_t m_offset;
  size_t m_size;
  std::string m_koohar_tag;
  std::string m_koohar_path_prop_name;

}; // class Parser

} // namespace xml

} // namespace koohar

#endif // koohar_parser_hh
