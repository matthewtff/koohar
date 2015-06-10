#ifndef koohar_regex_hh
#define koohar_regex_hh


// gcc (even v4.9) does not implement <regex> properly.
#if defined(__GNUC__) && !defined(__clang__)

#include <boost/regex.hpp>

namespace koohar {
namespace re {
using boost::match_results;
using boost::regex;

namespace regex_constants {
using match_flag_type = boost::regex_constants::match_flag_type;
using boost::regex_constants::match_default;
}  // namespace regex_constatns
using boost::regex_search;
}  // namespace re
}  // namespace koohar

#else  // defined(__GNUC__) && !defined(__clang__)

#include <regex>

namespace koohar {
namespace re {
using std::match_results;
using std::regex;

namespace regex_constants {
using match_flag_type = std::regex_constants::match_flag_type;
using std::regex_constants::match_default;
}  // namespace regex_constatns
using std::regex_search;
}  // namespace re
}  // namespace koohar

#endif  // defined(__GNUC__) && !defined(__clang__)

#endif  // koohar_regex_hh
