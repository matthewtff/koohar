#ifndef koohar_socket_hh
#define koohar_socket_hh

#ifdef _WIN32

#pragma comment(lib, "ws2_32.lib")

#include <winsock2.h>
#include <ws2tcpip.h>

#endif /* _WIN32 */

#include <string>

namespace koohar {

void initSockets();
void destroySockets();

#ifdef _WIN32

void close (SocketHandle sock);

#else /* _WIN32 */

enum { SOCKET_ERROR = -1 };

#endif /* _WIN32 */


/*
 * There're 2 types of sockets: listening and receiving/sending one (:
*/
class Socket {
public:
	typedef enum {
		NoError,
		PipeError,
		AgainError
	} Error;

#ifdef _WIN32

	typedef SOCKET Handle;

#else /* _WIN32 */

	typedef int Handle;

#endif /* _WIN32 */


public:
	Socket () {}
	Socket (const bool Async, const bool IPv4); // future listening socket
	Socket (const Handle Sock, const std::string& IP,
		const std::string& Port, const bool Async = true,
		const bool IPv4 = true); // receiving/sending socket

	void init ();
	bool listen (const std::string& Address = "0.0.0.0",
		const int Port = 7000, const int BackLog = 128);
	
	bool connect (const std::string& Address, const int Port);

	Socket accept () const;
	void close () const;
	Error getCh (char* Mem, const size_t Length, int &Readed);
	Handle fd () const { return m_socket; }
	std::string ip () const { return m_ip; }
	std::string port () const { return m_port; }

private:
	Handle m_socket;
	std::string m_ip;
	std::string m_port;
	bool m_async;
	bool m_ipv4;
}; // class Socket

}; // namespace koohar

#endif // koohar_socket_hh
