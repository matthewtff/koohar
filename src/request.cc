#include "request.hh"

#ifdef _WIN32

#include <winsock2.h>

#else /* _WIN32 */

#include <errno.h>

#endif /* _WIN32 */

namespace koohar {

std::string Request::m_session_error = "Session is not set.";

std::string& Request::session (const std::string& Key)
{
  return (!m_session)
    ? m_session_error
    : m_session->operator[](Key);
}

void Request::unsetSession (const std::string& Key)
{
  if (m_session)
    m_session->erase(Key);
}

bool Request::contains (const std::string& Url) const
{
  return uri().find(Url) != Url.npos;
}

bool Request::corresponds (const std::string& StaticUrl) const
{
  if (uri().empty() || uri().length() < StaticUrl.length())
    return false;
  return uri().find(StaticUrl) == 0;
}

} // namespace koohar
