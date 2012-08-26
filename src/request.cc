#ifdef _WIN32

#include <winsock2.h>

#else /* _WIN32 */

#include <errno.h>

#endif /* _WIN32 */

#include "request.hh"

namespace koohar {

std::string Request::m_session_error = "Session is not set.";

Request::Request ()
{
}

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

bool Request::contains (const std::string& Param) const
{
	if (Param.empty() || path().empty())
		return false;
	if (path().find(Param) == path().npos)
		return false;
	return true;
}

bool Request::corresponds (const std::string& Param) const
{
	if (Param.empty() || path().empty() || path().length() < Param.length())
		return false;
	for (size_t count = 0; count < Param.length(); ++count)
		if (Param[count] != path()[count])
			return false;
	return true;
}

bool Request::receive ()
{
	char buf[256];
	int readed = 0;
	Socket::Error sock_err = m_socket.getCh(buf, sizeof(buf), readed);
	switch (sock_err) {
		case Socket::AgainError: return false;
		case Socket::PipeError: return true;
		default: // SOCK_NO_ERROR
			break;
	}
	update(buf, readed); // HttpParser::Update
	return isComplete(); // HttpParser::isComplete
}

}; // namespace koohar
