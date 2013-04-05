#ifndef koohar_date_hh
#define koohar_date_hh

#include <string>
#include <boost/date_time/local_time/local_time.hpp>

namespace koohar {

class Date {

public:

	Date ();
	explicit Date (const std::string& DateString);

	std::string toString () const;

	void parse (const std::string& DateString);

private:

	boost::local_time::local_date_time m_date;

}; // class Date

} // namespace koohar

#endif // koohar_date_hh
