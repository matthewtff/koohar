#include <cstring>
#include <errno.h>
#include <signal.h>

#include "server.hh"

#ifdef _DEBUG
#include <iostream>
#endif /* _DEBUG */

namespace koohar {

#ifndef _WIN32

//static void sig_pipe(int) {} // just a fake to catch SIGPIPE and do nothing

#endif /* !_WIN32 */

Server::Server(const std::string& Address, const unsigned short Port,
	const bool IPv4, const int Backlog)
{
	initSockets();
	m_socket.init();
	m_address = Address;
	m_port = Port;
	m_ipv4 = IPv4;
	m_backlog = Backlog;
#ifndef _WIN32

	//if (signal(SIGPIPE, sig_pipe) == SIG_ERR) {
	if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
#ifdef _DEBUG
		// TODO: Create and use some log mechanism for such cases.
		std::cerr << "[Server::Server] Unable to catch signal SIGPIPE" << std::endl;
#endif /* _DEBUG */
	}

#endif /* !_WIN32 */
}

Server::~Server()
{
	stop();
	destroySockets();
}

} // namespace koohar
