#ifndef koohar_server_hh
#define koohar_server_hh

#include <string>

#include "socket.hh"

namespace koohar {

class Server
{
public:
	Server(const std::string& Address = "localhost",
		const unsigned short Port = 80, const bool IPv4 = true,
		const int Backlog = 128);
	~Server();
	int socket() const { return m_socket.fd(); }
	bool listen() { return m_socket.listen(m_address, m_port, m_backlog); }
	Socket accept() { return m_socket.accept(); }
	void port(const int NewPort) { m_port = NewPort; }
	unsigned short port() const { return m_port; }
	virtual void stop () { m_socket.close(); }

private:
	Socket m_socket;
	std::string m_address;
	unsigned short m_port;
	bool m_ipv4;
	int m_backlog;
}; // class Server

}; // namespace koohar

#endif // koohar_server_hh
