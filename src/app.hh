#ifndef koohar_app_hh
#define koohar_app_hh

#include <map>
#include <list>
#include <string>
#include <functional>
#include <memory>

// <mutex> header support was implemented since VS 2012 RC,
// so for now we still need boost.
#ifdef _WIN32

#include <boost/signals2/mutex.hpp>

namespace std {
	using boost::signals2::mutex;
}

#else /* _WIN32 */

#include <mutex>

#endif /* _WIN32 */

#include "server.hh"
#include "request.hh"
#include "response.hh"
#include "sender.hh"
#include "async.hh"
#include "date.hh"

#ifdef _DEBUG
#include <iostream>
#endif /* _DEBUG */

namespace koohar {

struct ReqRes {
	ReqRes (Sender& sender) : res(sender) {}
	Request req;
	Response res;
};

typedef std::function<void(koohar::Request&, koohar::Response&)> UserFunc;
typedef std::map<Async::Key, std::shared_ptr<ReqRes> > ConnectMap;

//StringMap defined in request.hh
typedef std::map<std::string, StringMap> SessionMap;

class App {
public:

	struct ThreadInfo {
		ThreadInfo (App& app, Async& async, ConnectMap& connections,
			UserFunc user_func, std::mutex& connect_mutex,
			std::mutex& thread_mutex):
			m_app(app), m_async(async), m_connections(connections),
			m_user_func(user_func), m_connect_mutex(connect_mutex),
			m_thread_mutex(thread_mutex)
		{}

	public:
		App& m_app;
		Async& m_async;
		ConnectMap& m_connections;
		UserFunc m_user_func;
		std::mutex& m_connect_mutex;
		std::mutex& m_thread_mutex;
	}; // struct ThreadInfo

	typedef std::list<std::string> StringList;

	enum { CookieLength = 64};

	// This value is used to trunc send data if it is large
	// We don't want to map 4GB to data per file
	enum { MAX_STATIC_SEND = 16777216}; // 16 MB

	static void generateCookieKey(std::string& PlaceTo);

public:
	App (const std::string& IP = "localhost", const unsigned short Port = 80,
		unsigned short ThreadCount = 1);
	~App () { close(); }
	void config (const std::string& ConfName, const std::string& ConfValue);
	void listen (UserFunc UserCallFunction);
	void registerMime (const std::string& FileExt, const std::string& Mime)
	{
		if (!FileExt.empty() && !Mime.empty())
			m_mime[FileExt] = Mime;
	}
	void cookieName(const std::string& CookieName)
	{
		if (!CookieName.empty())
			m_cookie_name = CookieName;
	}
	std::string cookieName () const  { return m_cookie_name; }
	StringMap& session (const std::string& SessionId) { return m_sessions[SessionId]; }
	// TODO: Move this method somewere far from here...
	bool transferStatic(Request& Req, Response& Res);
	void close() { m_server.stop(); }

private:
	Server m_server;
	Sender m_sender;
	std::string m_static_dir;
	std::string m_404_file;
	StringList m_static;
	StringMap m_mime;
	unsigned short m_thread_count;
	std::mutex m_connect_mutex;
	std::string m_cookie_name;
	SessionMap m_sessions;
}; // class App

}; // namespace koohar

#endif // koohar_app_hh
