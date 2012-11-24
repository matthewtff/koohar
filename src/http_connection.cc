#include "http_connection.hh"

#include <boost/bind.hpp>

#include "response.hh"
#include "file.hh"

using boost::asio::ip::tcp;

namespace koohar {

HttpConnection::Pointer HttpConnection::create (
	boost::asio::io_service& IoService, UserFunc UserCallFunction,
	const ServerConfig* Config)
{
	return Pointer(new HttpConnection(IoService, UserCallFunction,
		Config));
}

tcp::socket& HttpConnection::socket ()
{
	return m_socket;
}

void HttpConnection::start ()
{
	m_socket.async_read_some (
		boost::asio::buffer(m_request_buffer, MaxRequestSize),
		boost::bind(&HttpConnection::handleRead, shared_from_this(),
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred
		)
	);
}

void HttpConnection::write (const char* Data, size_t Size)
{
	if (!Data || !Size)
		return;

	boost::asio::async_write(m_socket, boost::asio::buffer(Data, Size),
		boost::bind(&HttpConnection::handleWrite, shared_from_this(),
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred
		)
	);
}

void HttpConnection::close ()
{
	m_socket.close();
}

void HttpConnection::setUserCallFunction (UserFunc UserCallFunction)
{
	m_user_call_function = UserCallFunction;
}

// private

HttpConnection::HttpConnection (boost::asio::io_service& IoService,
	UserFunc UserCallFunction, const ServerConfig* Config) :
	ServerConfig(*Config), m_socket(IoService),
	m_user_call_function(UserCallFunction)
{}

void HttpConnection::handleRead (const boost::system::error_code& Error,
	size_t BytesTransferred)
{
	if (Error)
		std::cout << "HttpConnection[read_error] : " << Error.message()
			<< std::endl;
	else {
		Response res(shared_from_this());

		if (!m_request.update(m_request_buffer, BytesTransferred)) {
			res.writeHead(m_request.errorCode());
			res.end();
		} else if (ServerConfig::isStaticUrl(m_request)) {
			transferStatic(res);
		} else if (m_user_call_function)
			m_user_call_function(m_request, res);
		else
			std::cout << "HttpConnection[callback_error] : "
				"Callback was not set" << std::endl;
	}
}

void HttpConnection::handleWrite (const boost::system::error_code& Error,
	size_t BytesTransferred)
{
	if (Error) {
		std::cout << "HttpConnection[write_error] : " << Error.message()
			<< std::endl;
	}
	else if (BytesTransferred == 0)
		std::cout << "HttpConnection[write_error] : transferred 0 bytes"
			<< std::endl;
}

void HttpConnection::transferStatic (Response& Res)
{
	std::string static_dir = getStaticDir();
	std::string file_name = static_dir.empty() ? "." : static_dir;
	file_name.append(m_request.uri());

	File static_file(file_name);

	bool vulnerability = (file_name.find("..") != file_name.npos)
		|| file_name[0] == '/';

	if (vulnerability || !static_file.open(File::ReadOnly)) {
#ifdef _DEBUG
		std::cout << "file not found : " << file_name << std::endl;
		std::cout << "error: " << strerror(errno) << std::endl;
#endif /* _DEBUG */
		Res.writeHead(404);
		Res.end("<html><body><h1>kooher</h1><h2>Page not found</h2>"
			"</body></html>");
		return;
	}

	Res.writeHead(200);
	Res.sendFile(file_name.c_str(), static_file.getSize(), 0);
	Res.end();
}

} // namespace koohar
