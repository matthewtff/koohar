#ifndef koohar_socket_hh
#define koohar_socket_hh

#ifdef _WIN32

#pragma comment(lib, "ws2_32.lib")

#include <winsock2.h>
#include <ws2tcpip.h>

#endif /* _WIN32 */

#include <string>

namespace koohar {

// Windows sockets cant live without these dirty hacks....
void initSockets();
void destroySockets();

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
		int Port, const bool Async = true,
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
	int port () const { return m_port; }

private:
	Handle m_socket;
	std::string m_ip;
	int m_port;
	bool m_async;
	bool m_ipv4;
}; // class Socket

#ifdef _WIN32

void close ( Socket::Handle sock);

#else /* _WIN32 */

enum { SOCKET_ERROR = -1 };

#endif /* _WIN32 */

}; // namespace koohar

#endif // koohar_socket_hh
