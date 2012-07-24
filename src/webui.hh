#ifndef koohar_webui_hh
#define koohar_webui_hh

#include <map>
#include <list>
#include <string>
#include <algorithm>
#include <memory>

namespace koohar {

namespace webui {

class Object;

typedef std::shared_ptr<Object> ObjectPtr;
typedef std::map<std::string, std::string> StringMap;

class Object : public StringMap {
public:
	Object (const std::string& SearchRule, const std::string& SomeTag = "");
	virtual ~Object() {}
	void tag(const std::string& NewTag) { m_tag = NewTag; }
	std::string tag() const { return m_tag; }
	std::string& set (const std::string& Prop);
	bool appendChild(ObjectPtr Child);

	// Execute predicate on every child
	template<class Predicate> void everyChild(const Predicate& Pred)
	{
		std::for_each(m_children.begin(), m_children.end(), Pred);
	}

private:
	std::string m_search_rule;
	std::string m_tag;
	std::list<ObjectPtr> m_children;
}; // class WebObject

}; // namespace web_ui

}; // namespace koohar

#endif // koohar_webui_hh
