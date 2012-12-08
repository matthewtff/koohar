#include "date.hh"

#include <boost/date_time.hpp>
//#include <boost/date_time/local_time/local_time.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <exception>
#include <sstream>

namespace koohar {

Date::Date () : m_date(boost::local_time::local_sec_clock::local_time(
	boost::local_time::time_zone_ptr()
))
{
}

Date::Date (const std::string& DateString) : m_date(
	boost::local_time::local_sec_clock::local_time(
		boost::local_time::time_zone_ptr()
	)
)
{
	parse(DateString);
}

std::string Date::toString () const
{
	std::stringstream stream;
	boost::local_time::local_time_facet* local_facet =
		new boost::local_time::local_time_facet("%a, %d %b %Y %H:%M:%S GMT");
	
	try {
		stream.imbue(std::locale(stream.getloc(), local_facet));
		stream << m_date;
	} catch (std::exception& e) {
		return std::string();
	}

	return stream.str();
}

void Date::parse (const std::string& DateString)
{
	boost::local_time::local_time_input_facet* local_facet =
		new boost::local_time::local_time_input_facet(
			"%a, %d %b %Y %H:%M:%S GMT"
		);
	
	std::stringstream stream(DateString);
	stream.imbue(std::locale(std::locale::classic(), local_facet));
	stream >> m_date;
}

} // namespace koohar
