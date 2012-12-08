#ifndef koohar_http_connection_hh
#define koohar_http_connection_hh

#include <string>
#include <memory>
#include <list>
#include <map>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <boost/asio.hpp>

#include "server_config.hh"
#include "request.hh"

namespace koohar {

class Response;

class HttpConnection :
	public ServerConfig,
	public boost::enable_shared_from_this< HttpConnection >
{
public:
	enum { MaxRequestSize = 65536 };

	typedef boost::shared_ptr< HttpConnection > Pointer;

	typedef std::function< void (koohar::Request&, koohar::Response&) >
		UserFunc;

	typedef std::list< std::shared_ptr< const char > > DataBuffers;

	typedef std::map< std::string, std::string > StringMap;

public:
	static Pointer create (boost::asio::io_service& IoService,
		UserFunc UserCallFunction, const ServerConfig* Config);

	boost::asio::ip::tcp::socket& socket ();

	void start ();

	void write (const char* Data, size_t Size);

	void close ();

	void setUserCallFunction (UserFunc UserCallFunction);

	static StringMap initMimeTypes ();

public:

	static StringMap m_mime_types;

private:
	HttpConnection (boost::asio::io_service& IoService,
		UserFunc UserCallFunction, const ServerConfig* Config);

	void handleRead (const boost::system::error_code& Error,
		size_t BytesTransferred);

	void handleWrite (const boost::system::error_code& Error,
		size_t BytesTransferred);

	void transferStatic (Response& Res);

private:
	boost::asio::ip::tcp::socket m_socket;
	std::string m_answer;
	char m_request_buffer[MaxRequestSize];

	Request m_request;
	UserFunc m_user_call_function;
	DataBuffers m_buffers;
	int m_writing_operations;
	bool m_close_socket;
};

} // namespace koohar

#endif // koohar_http_connection_hh
