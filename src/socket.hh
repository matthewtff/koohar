#ifndef koohar_socket_hh
#define koohar_socket_hh

#ifdef _WIN32

#pragma comment(lib, "ws2_32.lib")

#include <winsock2.h>
#include <ws2tcpip.h>

typedef SOCKET SocketHandle;

#else /* _WIN32 */

//#define SocketHandle int
typedef int SocketHandle;

#endif /* _WIN32 */

#include <string>

namespace koohar {

void initSockets();
void destroySockets();

#ifdef _WIN32

void close (SocketHandle sock);

#endif /* _WIN32 */


typedef enum
{
	SOCK_NO_ERROR,
	SOCK_ERROR,
	SOCK_AGAIN_ERROR
} SocketErrorType;
/*
 * There're 2 types of sockets: listening and receiving/sending one (:
*/
class Socket {
public:
	Socket () {}
	Socket (const bool Async, const bool IPv4); // receiving socket made by listening one
	Socket (const SocketHandle Sock, const std::string& IP,
		const std::string& Port, const bool Async = true,
		const bool IPv4 = true); // listening socket constructor

	void init ();
	bool listen (const std::string& Address = "0.0.0.0",
		const int Port = 7000, const int BackLog = 128);

	Socket accept () const;
	void close () const;// { ::close(m_socket); }
	SocketErrorType getCh (char* Mem, const size_t Length, int &Readed);
	SocketHandle fd () const { return m_socket; }
	std::string ip () const { return m_ip; }
	std::string port () const { return m_port; }

private:
	SocketHandle m_socket;
	std::string m_ip;
	std::string m_port;
	bool m_async;
	bool m_ipv4;
	bool m_listening; // true if socket is listening
}; // class Socket

}; // namespace koohar

#endif // koohar_socket_hh
