#include "socket.hh"

#ifdef _WIN32

#else /* _WIN32 */

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <fcntl.h>

#endif /* _WIN32 */

#include <errno.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>

#ifdef _DEBUG
#include <iostream>
#endif /* _DEBUG */

namespace koohar {

void initSockets ()
{
#ifdef _WIN32
	WSADATA ws_start_data;
	WSAStartup(MAKEWORD(2, 0), &ws_start_data);
#endif /* _WIN32 */
}

void destroySockets ()
{
#ifdef _WIN32
	WSACleanup();
#endif /* _WIN32 */
}

#ifdef _WIN32

void close (SocketHandle sock)
{
	closesocket(sock);
}

#else /* _WIN32 */

void setnonblocking(const int Sock)
{
	int opts;
	opts = fcntl(Sock, F_GETFL);
	if (opts < 0) {
#ifdef _DEBUG
		std::cout << "Error getting flags." << std::endl;
#endif /* _DEBUG */
		return;
	}
	opts = (opts | O_NONBLOCK);
	if (fcntl(Sock, F_SETFL, opts) < 0) {
#ifdef _DEBUG
		std::cout << "Error setting flags." << std::endl;
#endif /* _DEBUG */
		return;
	}
	return;
}

#endif /* _WIN32 */

Socket::Socket (const bool Async, const bool IPv4) :
	m_async(Async), m_ipv4(IPv4)
{
	init();
}

Socket::Socket (const Handle Sock, const std::string& IP,
	const std::string& Port, const bool Async, const bool IPv4) :
	m_socket(Sock), m_ip(IP), m_port(Port), m_async(Async), m_ipv4(IPv4)
{
#ifndef _WIN32

	// For now let it be blocking...
	if (m_async)
		setnonblocking(Sock);

#endif /* !_WIN32 */
}

void Socket::init()
{
#ifdef _WIN32

	//m_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	m_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);
	if (m_socket == INVALID_SOCKET) {
#ifdef _DEBUG
		std::cerr << "Invalid socket" << std::endl;
#endif /* _DEBUG */
		DWORD err = GetLastError();
#ifdef _DEBUG
		char msg[2000];
		FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), msg, 2000, NULL);
		std::cout << msg;
#endif /* _DEBUG */
		return;
	}

#else /* _WIN32 */

	if ((m_socket = ::socket(AF_INET, SOCK_STREAM, 0)) < 0) {
#ifdef _DEBUG
		std::cout << "Error creating socket:" << strerror(errno) << std::endl;
#endif /* _DEBUG */
		return;
	}
	int reuse = 1;
	setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

#endif /* _WIN32 */
}

bool Socket::listen (const std::string& Address, const int Port, const int BackLog)
{
	m_ip = Address;
	char port_str[6];
	sprintf(port_str, "%d", Port);
	m_port = port_str;

	sockaddr_in sa;
	memset(&sa, 0, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_port = htons(Port);
	sa.sin_addr.s_addr = INADDR_ANY;
	if (bind(m_socket, reinterpret_cast<struct sockaddr*>(&sa), sizeof(sockaddr)) < 0) {
#ifdef _DEBUG
		std::cout << "Error binding socket: " << strerror(errno) << std::endl;
#endif /* DEBUG */
		return false;
	}
	return ::listen(m_socket, BackLog) != -1;
}

bool Socket::connect (const std::string& Address, const int Port)
{
	m_ip = Address;
	char port_str[6];
	sprintf(port_str, "%d", Port);
	m_port = port_str;

	sockaddr_in sa;
	memset(&sa, 0, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = inet_addr(m_ip.c_str());
	sa.sin_port = htons(Port);
	if (::connect(m_socket, reinterpret_cast<struct sockaddr*>(&sa),
		sizeof(sa)) == SOCKET_ERROR)
	{
#ifdef _DEBUG
		std::cout << "Error connecting to " << Address << " :"
			<< strerror(errno) << std::endl;
#endif /* _DEBUG */
		return false;
	}
	return true;
}

Socket Socket::accept () const
{
	sockaddr addr;
	socklen_t address_size = sizeof(addr);
	Handle accept_sock;
	if ((accept_sock = ::accept(m_socket, &addr, &address_size)) < 0) { // TODO: also use exception
#ifdef _DEBUG
		std::cout << "Error accepting socket: " << strerror(errno) << std::endl;
#endif /* _DEBUG */
	}
	// these options are also not important to be set, so no return value check.
	// actually i really don't know why this code is here.
	// but it is kinda well comented, so just leave it.
	/*const timeval wait_timeout = {0, 2000}; // timeout on recieve data from socket
	setsockopt(accept_sock, SOL_SOCKET, SO_RCVTIMEO, &wait_timeout, sizeof(wait_timeout));
	const linger close_timeout = {0, 1}; // unset timeout to close socket while there is data to be sent
	setsockopt(accept_sock, SOL_SOCKET, SO_LINGER, &close_timeout, sizeof(close_timeout));*/
	// fill in info about client
	char port[6];
	char ip[INET6_ADDRSTRLEN]; // we choose ipv6 cause INET6_ADDRSTRLEN > INET_ADDRSTRLEN
	if (addr.sa_family == AF_INET) {
		sockaddr_in* address = reinterpret_cast<sockaddr_in*>(&addr);
		sprintf(port, "%d", address->sin_port);
#ifdef _WIN32

		getnameinfo(&addr, sizeof(addr), ip, INET_ADDRSTRLEN, NULL, 0, NI_NUMERICHOST);

#else /* _WIN32 */

		inet_ntop(AF_INET, reinterpret_cast<void*>(&address->sin_addr.s_addr), ip, INET_ADDRSTRLEN); // TODO: check for error and use exception
		
#endif /* _WIN32 */
	} else {
		sockaddr_in6* address = reinterpret_cast<sockaddr_in6*>(&addr);
		sprintf(port, "%d", address->sin6_port);
#ifdef _WIN32

		getnameinfo(&addr, sizeof(addr), ip, INET6_ADDRSTRLEN, NULL, 0, NI_NUMERICHOST);

#else /* _WIN32 */

		inet_ntop(AF_INET6, reinterpret_cast<void*>(&address->sin6_addr.s6_addr), ip, INET6_ADDRSTRLEN); // TODO: same here

#endif /* _WIN32 */
	}
	return Socket(accept_sock, ip, port, m_async, m_ipv4);
}

void Socket::close () const
{
#ifdef _WIN32
	closesocket(m_socket);
#else /* _WIN32 */
	::close(m_socket);
#endif /* _WIN32 */
}

Socket::Error Socket::getCh (char* Mem, const size_t Length, int &Readed)
{
#ifdef _WIN32

	DWORD readed_data = 0;
	/*
	LPOVERLAPPED ovr = new OVERLAPPED;
	ovr->Offset = 0;
	ovr->OffsetHigh = 0;
	ovr->hEvent = NULL;
	*/

	if (!ReadFile((HANDLE)m_socket, Mem, Length, &readed_data, NULL)) {
		Readed = static_cast<int>(readed_data);
		if (GetLastError() != ERROR_HANDLE_EOF)
			return AgainError;
#ifdef _DEBUG
		char msg[2000];
		FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), msg, 2000, NULL);
		std::cerr << msg;
#endif /* _DEBUG */
		return PipeError;
	}
	Readed = static_cast<int>(readed_data);

#else /* _WIN32 */

	int readed_data = 0;
	readed_data = read(m_socket, Mem, Length);
	Readed = readed_data;
	if (readed_data < 0) {
		if (errno == EAGAIN)
			return AgainError;
		else
			return PipeError;
	}

#endif /* _WIN32 */
	return readed_data ? NoError : PipeError;
}

}; // namespace koohar
