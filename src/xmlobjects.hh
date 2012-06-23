#ifndef koohar_xmlobjects_hh
#define koohar_xmlobjects_hh

#include <string>
#include <list>

#include <ostream>

namespace koohar {

namespace xml {

class Property {
public:
	Property (const std::string& NewName = "", const std::string& NewValue = "")
		: m_name(NewName), m_value(NewValue) {}
	void name(const std::string& SomeName) { m_name = SomeName; }
	std::string& name() { return m_name; }
	const std::string& name() const { return m_name; }
	void value(const std::string& SomeValue) { m_value = SomeValue; }
	std::string& value() { return m_value; }
	const std::string& value () const { return m_value; }
	void print(std::ostream& out) { out << " " << m_name << "=\"" << m_value << "\""; }
	bool operator == (const Property& That) { return !m_name.compare(That.m_name); }
private:
	std::string m_name;
	std::string m_value;
}; // class Property

class Object {
public:
	Object (const std::string& NewTag = "") : m_tag(NewTag) {}	
	void tag(const std::string& SomeTag) { m_tag = SomeTag; }	
	const std::string& tag() const { return m_tag; }
	std::string& tag() { return m_tag; }
	const std::string text() const;

	// Allows to get user-specified property
	template<class Predicate> Property* property(const Predicate& Pred)
	{
		for (auto it = m_properties.begin();
			it != m_properties.end(); ++it)
		{
			if (Pred(*it))
				return &(*it);
		}
		return 0;
	}

	// Add property
	void property(const Property& Prop);

	// Allows to get user-specified object
	template<class Predicate> Object* object(const Predicate& Pred)
	{
		for (auto it = m_objects.begin(); it != m_objects.end(); ++it)
			if (Pred(*it))
				return &(*it);
		return 0;
	}

	// Add object
	void object(const Object& Obj) { m_objects.push_back(Obj); }

	template<class Predicate> void properties(const Predicate& Pred)
	{
		// Sadly, but std::for_each copies Predicate :(
		for (auto it = m_properties.begin(); it != m_properties.end(); ++it)
			Pred(*it);
	}

	template<class Predicate> void objects(const Predicate& Pred)
	{
		for (auto it = m_objects.begin();
			it != m_objects.end(); ++it)
		{
			Pred(*it);
			it->objects(Pred);
		}
	}

	void print(std::ostream& out, const std::string& koohar_tag, size_t tab_number = 0);
	void printIntro(std::ostream& out, const std::string& koohar_tag, size_t tab_number = 0);
	void clear ();

	// Copies file to some dom objects. Used to include one file from enother
	void copy (Object& CopyTo);

private:
	std::string m_tag;
	std::list<Property> m_properties;
	std::list<Object> m_objects;

	struct SameName {
		SameName (const std::string& Name) : m_name(Name) {}
		bool operator() (const Property& Prop) { return !Prop.name().compare(m_name); }
	private:
		std::string m_name;
	}; // struct RemoveProperty

}; // class Object

}; // namespace xml

}; // namespace koohar

#endif // koohar xmlobjects_hh
