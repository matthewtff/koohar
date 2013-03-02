#include "http_connection.hh"

#include <boost/bind.hpp>

#include "response.hh"
#include "file.hh"

#include <sstream>

#include <iostream>

using boost::asio::ip::tcp;

namespace koohar {

HttpConnection::StringMap HttpConnection::initMimeTypes ()
{
	StringMap mimes;
	mimes["png"] = "image/png";
	mimes["jpg"] = "image/jpeg";
	mimes["gif"] = "image/gif";
	mimes["peg"] = "image/jpeg";
	mimes["jpe"] = "image/jpeg";
	mimes["tif"] = "image/tiff";
	mimes["iff"] = "image/tiff";
	mimes["htm"] = "text/html";
	mimes["tml"] = "text/html";
	mimes["txt"] = "text/plain";
	mimes["css"] = "text/css";
	mimes["rtx"] = "text/richtext";
	mimes["pdf"] = "application/pdf";
	mimes["rtf"] = "application/rtf";
	mimes["zip"] = "application/zip";
	mimes["wav"] = "application/x-wav";
	mimes["mka"] = "audio/x-matroska";
	mimes["peg"] = "video/mpeg";
	mimes["mpg"] = "video/mpeg";
	mimes["mpe"] = "video/mpeg";
	mimes["mkv"] = "video/x-matroska";
	mimes["ebm"] = "video/webm"; // webm actually
	mimes["mov"] = "video/quicktime";
	mimes[".js"] = "application/x-javascript";
	mimes["k3d"] = "video/x-matroska-3d"; // mk3d actually
	mimes["son"] = "application/json"; // json actually
	return mimes;
}

StringMap HttpConnection::m_mime_types = HttpConnection::initMimeTypes();
	
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

	std::shared_ptr<char> buffer (new char[Size]);

	std::memcpy(buffer.get(), Data, Size);

	boost::asio::async_write(m_socket, boost::asio::buffer(buffer.get(), Size),
		boost::bind(&HttpConnection::handleWrite, shared_from_this(),
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred
		)
	);

	m_buffers.push_back(buffer);

	++m_writing_operations;
}

void HttpConnection::close ()
{
	m_close_socket = true;
}

void HttpConnection::setUserCallFunction (UserFunc UserCallFunction)
{
	m_user_call_function = UserCallFunction;
}

// private

HttpConnection::HttpConnection (boost::asio::io_service& IoService,
	UserFunc UserCallFunction, const ServerConfig* Config) :
	ServerConfig(*Config), m_socket(IoService),
	m_user_call_function(UserCallFunction),
	m_writing_operations(0),
	m_close_socket(false)
{}

void HttpConnection::handleRead (const boost::system::error_code& Error,
	size_t BytesTransferred)
{
	if (Error) {
		/*std::cout << "HttpConnection[read_error] : " << Error.message()
			<< std::endl;*/
	} else {
		Response res(shared_from_this());

		res.header("Server", "koohar.app");
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
		/*std::cout << "HttpConnection[write_error] : " << Error.message()
			<< std::endl;*/
	}
	else if (BytesTransferred == 0) {
		std::cout << "HttpConnection[write_error] : transferred 0 bytes"
			<< std::endl;
	}

	if (m_writing_operations > 0)
		--m_writing_operations;
	if (m_close_socket && m_writing_operations == 0)
		m_socket.close();
}

void HttpConnection::transferStatic (Response& Res)
{
	std::string static_dir = getStaticDir();
	std::string file_name = static_dir.empty() ? "." : static_dir;
	file_name.append(m_request.uri());

	if (File::isDirectory(file_name.c_str())) {
		auto error_page = getErrorPage(400);
		error_page.empty() ? Res.writeHead(400) : Res.redirect(error_page);
		Res.end();
	}

	File static_file(file_name);

	bool vulnerability = (file_name.find("..") != file_name.npos)
		|| file_name[0] == '/';

	if (vulnerability || !static_file.open(File::ReadOnly)) {
#ifdef _DEBUG
		std::cout << "file not found : " << file_name << std::endl;
		std::cout << "error: " << strerror(errno) << std::endl;
#endif /* _DEBUG */
		std::string error_page = getErrorPage(404);
		error_page.empty() ? Res.writeHead(404) : Res.redirect(error_page);
		Res.end();
		return;
	}

	Res.header("Connection", "Close");
	std::string mime = m_mime_types[file_name.substr(file_name.length() - 3, 3)];
	Res.header("Content-Type", mime);


	unsigned long shift = 0;
	unsigned long size = 0;
	// Check if header "Range: bytes=xxx-yyy" set up.
	if (!m_request.header("range").empty()
		|| static_file.getSize() > MaxStaticSize)
	{
		std::string range_header = m_request.header("range");
		if (!range_header.empty()) {
			const std::size_t eq_pos = range_header.find("=");
			const std::size_t sep_pos = range_header.find("-");
			if (eq_pos == range_header.npos || sep_pos == range_header.npos
				|| sep_pos < eq_pos)
			{ // Something really wierd with range.
				std::string error_page = getErrorPage(416);
				error_page.empty()
					? Res.writeHead(416)
					: Res.redirect(error_page);
				Res.end();
				return;
			}
			std::string start = range_header.substr(eq_pos + 1,
				sep_pos - eq_pos - 1);
			std::string end = range_header.substr(sep_pos + 1,
				range_header.length() - sep_pos);
			if (start.empty()) {
				size = std::atol(end.c_str());
				shift = static_file.getSize() - size;
			} else {
				shift = std::atol(start.c_str());
				size = end.empty()
					? (static_file.getSize() - shift)
					: (std::atol(end.c_str()) - shift);
			}
			if (shift + size > static_file.getSize()) {
				std::string error_page = getErrorPage(416);
				error_page.empty()
					? Res.writeHead(416)
					: Res.redirect(error_page);
				Res.end();
				return;
			}
		}
		size = size ? size : static_file.getSize();
		size = size > MaxStaticSize ? MaxStaticSize : size;
		Res.writeHead(206);
		std::stringstream range;
		range << "bytes " << shift << "-" << (size + shift - 1)
			<< "/" << static_file.getSize();
		Res.header("Content-Range", range.str().c_str());
	} else {
		std::stringstream length;
		length << static_file.getSize();
		Res.header("Content-Length", length.str());
		Res.writeHead(200);
	}

	Res.sendFile(static_file.getHandle(), static_file.getSize(), 0);
	Res.end();
}

} // namespace koohar
