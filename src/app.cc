#include <cstdio>
#include <errno.h>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <vector>

#include "app.hh"
#include "date.hh"
#include "file.hh"
#include "thread.hh"

#ifdef _DEBUG
#include <iostream>
#endif /* _DEBUG */

using namespace std;

namespace koohar {

enum { CookieLength = 64};

// This value is used to trunc send data if it is large
// We don't want to map 4GB to data per file
enum { MAX_STATIC_SEND = 16777216}; // 16 MB

void generateCookieKey(std::string& PlaceTo)
{
	PlaceTo.erase();
	for (size_t count = 0; count < CookieLength; ++count)
		PlaceTo.append(1, static_cast<char>(62 + rand() % 60)); // ord(62)-'a'
}

ThreadReturnValue listening_thread(ThreadGetValue TInfo)
{
	if (!TInfo) // Wanna be sure its OK.
		return 0;

	// Theoretically pointer can get bad anytime, so we use handle instead.
	App::ThreadInfo& thread_info = *(reinterpret_cast<App::ThreadInfo*>(TInfo));
	
	thread_info.m_thread_mutex.lock();
	while (true) {
		Async::Key key = thread_info.m_async.get();

		thread_info.m_connect_mutex.lock();
		ConnectMap::iterator conn_it = thread_info.m_connections.find(key);
		if (conn_it == thread_info.m_connections.end()) {
			thread_info.m_connect_mutex.unlock();
			continue;
		}
		shared_ptr<ReqRes> reqres = conn_it->second;
		if (!reqres) {
			thread_info.m_connect_mutex.unlock();
			continue;
		}
		if (reqres->req.receive()) {
			thread_info.m_connections.erase(key);
			thread_info.m_connect_mutex.unlock();

			// let other threads work while we process the request
			thread_info.m_thread_mutex.unlock();

			if (!reqres->req.isBad()) { // check if there were any errors
				reqres->res.socket(reqres->req.socket());

				std::string sess_id = reqres->req.cookie(thread_info.m_app.cookieName());
				if (sess_id.empty()) {
					generateCookieKey(sess_id);
					reqres->res.cookie(thread_info.m_app.cookieName(), sess_id);
				}
				reqres->req.session( &(thread_info.m_app.session(sess_id)) );

				// If it is not static content : call user defined function.
				if (!thread_info.m_app.transferStatic(reqres->req, reqres->res))
					thread_info.m_user_func(reqres->req, reqres->res, thread_info.m_user_struct);

			} else { // if was - send bad request
				reqres->res.writeHead(reqres->req.errorCode());
				reqres->res.end();
			}
			thread_info.m_thread_mutex.lock();
		} else {
			thread_info.m_async.append(reqres->req.socket().fd());
			thread_info.m_connect_mutex.unlock();
		}
	}
	return 0; // never gets here ?
}

App::App (const std::string& IP, const unsigned short Port,
	const unsigned short ThreadCount) : m_server(IP, Port), m_404_file(""),
	m_thread_count(ThreadCount), m_cookie_name("koohar_cookie")
{
	srand(static_cast<unsigned int>(time(NULL)));
	if (!m_server.listen())
		std::cout << "Error starting server\n\r";
	m_mime["png"] = "image/png";
	m_mime["jpg"] = "image/jpeg";
	m_mime["gif"] = "image/gif";
	m_mime["peg"] = "image/jpeg";
	m_mime["jpe"] = "image/jpeg";
	m_mime["tif"] = "image/tiff";
	m_mime["iff"] = "image/tiff";
	m_mime["htm"] = "text/html";
	m_mime["tml"] = "text/html";
	m_mime["txt"] = "text/plain";
	m_mime["css"] = "text/css";
	m_mime["rtx"] = "text/richtext";
	m_mime["pdf"] = "application/pdf";
	m_mime["rtf"] = "application/rtf";
	m_mime["zip"] = "application/zip";
	m_mime["wav"] = "application/x-wav";
	m_mime["mka"] = "audio/x-matroska";
	m_mime["peg"] = "video/mpeg";
	m_mime["mpg"] = "video/mpeg";
	m_mime["mpe"] = "video/mpeg";
	m_mime["mkv"] = "video/x-matroska";
	m_mime["ebm"] = "video/webm"; // webm actually
	m_mime["mov"] = "video/quicktime";
	m_mime[".js"] = "application/x-javascript";
	m_mime["k3d"] = "video/x-matroska-3d"; // mk3d actually
}

void App::config (const std::string& ConfName, const std::string& ConfValue)
{
	if (ConfName.empty() || ConfValue.empty())
		return;
	if (!ConfName.compare("static_dir")) {
		m_static_dir = ConfValue;
	} if (!ConfName.compare("404 file")) {
		m_404_file = ConfValue;
	} else if (!ConfName.compare("static")) {
		m_static.push_back(ConfValue);
		m_static.unique();
	}
}

void App::listen (UserFunc UserCallFunction, void* UserStruct)
{
	Async connect_async(m_thread_count + 1);
	ConnectMap connections;

	// Starting threads
	std::mutex thread_mutex;
	std::vector<ThreadHandle> listen_threads(m_thread_count, ThreadHandle());
	std::list<ThreadInfo> thread_info;
	for (size_t count = 0; count < m_thread_count; ++count){
		thread_info.push_back( ThreadInfo( *this, connect_async, connections,
								UserCallFunction, UserStruct, m_connect_mutex,
								thread_mutex)
		);
		// We need to get adress of ThreadInfo stored right in std::list, so
		// get an iterator to last(just added) element using reverse begin.
		std::list<ThreadInfo>::reverse_iterator it = thread_info.rbegin();
		createThread(listen_threads[count], listening_thread,
			reinterpret_cast<ThreadGetValue>(&(*it))); // And now get adress.
	}

	// Actually listening loop that adds all new connections to epoll
	while (true) {
		shared_ptr<ReqRes> reqres(new ReqRes(m_sender));
		reqres->req.socket(m_server.accept());

		m_connect_mutex.lock();
		connections[reqres->req.socket().fd()] = reqres;
		m_connect_mutex.unlock();
		connect_async.append(reqres->req.socket().fd());
	}

	for (size_t count = 0; count < m_thread_count; ++count)
		joinThread(listen_threads[count]);
}

void App::registerMime(const std::string& FileExt, const std::string& Mime)
{
	if (!FileExt.empty() && !Mime.empty())
		m_mime[FileExt] = Mime;
}

void App::cookieName(const std::string& CookieName)
{
	if (!CookieName.empty())
		m_cookie_name = CookieName;
}

bool App::transferStatic (Request& Req, Response& Res)
{
	for (auto anyStatic = m_static.begin();
		anyStatic != m_static.end(); ++anyStatic)
	{
		if (!Req.corresponds(*anyStatic)) // We aren't interested in that.
			continue;
		std::string file_name(m_static_dir);
		file_name.append(Req.path());
		File static_file (file_name.c_str());
		bool vulnerability = (file_name.find("../") != file_name.npos)
			|| (file_name[0] == '/');
		if (vulnerability || !static_file.open(File::ReadOnly)) { // file not found or some other error ;)
#ifdef _DEBUG
			std::cerr << "file not found : " << file_name << std::endl;
			std::cerr << "error: " << strerror(errno) << std::endl;
#endif /* _DEBUG */
			Res.writeHead(404);
			if (m_404_file.empty()) {
				// TODO: User should be allowed to set his own 404 page...
				std::string not_found("<html><body><h1>kooher</h1><h2>Page not found</h2></body></html>");
				Res.end(not_found.c_str(), not_found.length());
			} else {
				Res.sendFile(m_404_file.c_str(), 0, 0);
				Res.end();
			}
			return true;
		}
		//Res.header("Expires", Date().incHours(2).toString());
		Res.header("Connection", "Close");

#ifdef __LP64__
		Res.header("Server", "koohar.app x86_64 v0.01");
#else /* __LP64__ */
		Res.header("Server", "koohar.app x86 v0.01");
#endif /* __LP64__ */

		Res.header("Last-Modified", Date(static_file.time()).toString());
		Date hreader_since(Req.header("if-modified-since"));
		if (hreader_since.time() != -1) // client sent 'If-Modified-Since' header, so check for it
			if (hreader_since.time() >= static_file.time()) { // content did not change
				Res.writeHead(304);
				Res.end();
				return true;
			}

		// Calculate mime.
		std::string mime = m_mime[file_name.substr(file_name.length() - 3, 3)];
		Res.header("Content-Type", mime);
		long long shift = 0;
		long long size = 0;
		if (!Req.header("range").empty() || static_file.size() > MAX_STATIC_SEND) { // "Range: bytes=xxx-yyy" => partial content
			if (!Req.header("range").empty()) {
				const size_t eq_pos = Req.header("range").find('=');
				const size_t sep_pos = Req.header("range").find('-');
				if (eq_pos == Req.header("range").npos
					|| sep_pos == Req.header("range").npos
					|| sep_pos < eq_pos)
				{ // something wrong with range
					Res.writeHead(416);
					Res.end();
					return true;
				}
				std::string start = Req.header("range").substr(eq_pos + 1, sep_pos - eq_pos - 1);
				std::string end = Req.header("range").substr(sep_pos + 1, Req.header("range").length() - sep_pos);
				if (start.empty()) {
					size = atol(end.c_str());
					shift = static_file.size() - size;
				} else {
					shift = atol(start.c_str());
					size = end.empty()
						? (static_file.size() - shift)
						: (atol(end.c_str()) - shift);
				}
				if (static_cast<size_t>(shift + size) > static_file.size()) {
					Res.writeHead(416); // Range Not Satisfiable.
					Res.end();
					return true;
				}
			}
			size = size ? size : static_file.size();
			size = size > static_cast<long long>(MAX_STATIC_SEND) ? static_cast<long long>(MAX_STATIC_SEND) : size;

			Res.writeHead(206);
			std::string range("bytes ");
			char num_container[100];
			sprintf(num_container, "%llu", static_cast<long long>(shift));
			range.append(num_container).append("-");
			sprintf(num_container, "%llu", static_cast<long long>(size + shift - 1));
			range.append(num_container).append("/");
			sprintf(num_container, "%llu", static_cast<long long>(static_file.size()));
			range.append(num_container);
			Res.header("Content-Range", range);
		} else
			Res.writeHead(200);

		char info_str[100];
		sprintf(info_str, "%d", static_cast<int>((size == 0) ? static_file.size() : size)); // get length
		Res.header("Content-Length", info_str);
		
		off_t actual_size = static_cast<off_t>(size ? size : static_file.size() - shift);
		Res.sendFile (file_name.c_str(), actual_size, static_cast<off_t>(shift));
		Res.end();
		return true;
	}
	
	return false; // never get here
}

}; // namespace koohar
