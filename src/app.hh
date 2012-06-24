#ifndef koohar_app_hh
#define koohar_app_hh

#include <map>
#include <list>
#include <string>

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

namespace koohar {

struct ReqRes {
	ReqRes (Sender& sender) : res(sender) {}
	Request req;
	Response res;
};

typedef void (*UserFunc)(Request &Req, Response &Res, void* UserStruct);
typedef std::map<AsyncKey, std::shared_ptr<ReqRes> > ConnectMap;

//StringMap defined in request.hh
typedef std::map<std::string, StringMap> SessionMap;

class App {
public:

	struct ThreadInfo {
		ThreadInfo (App& app, Async& async, ConnectMap& connections,
			UserFunc user_func, void* user_struct,
			std::mutex& connect_mutex,
			std::mutex& thread_mutex):
			m_app(app), m_async(async), m_connections(connections),
			m_user_func(user_func), m_user_struct(user_struct),
			m_connect_mutex(connect_mutex), m_thread_mutex(thread_mutex) {}

	public:
		App& m_app;
		Async& m_async;
		ConnectMap& m_connections;
		UserFunc m_user_func;
		void* m_user_struct;
		std::mutex& m_connect_mutex;
		std::mutex& m_thread_mutex;
	}; // struct ThreadInfo

	typedef std::list<std::string> StringList;

public:
	App (const std::string& IP = "localhost", const unsigned short Port = 80,
		unsigned short ThreadCount = 1);
	~App () { close(); }
	void config (const std::string& ConfName, const std::string& ConfValue);
	void listen (UserFunc UserCallFunction, void* UserStruct);
	void registerMime (const std::string& FileExt, const std::string& Mime);
	void cookieName(const std::string& CookieName);
	std::string cookieName () const  { return m_cookie_name; }
	StringMap& session (const std::string& SessionId) { return m_sessions[SessionId]; }
	bool transferStatic(Request& Req, Response& Res);
	void close() { m_server.stop(); }
private:
	Server m_server;
	Sender m_sender;
	std::string m_static_dir;
	StringList m_static;
	StringMap m_mime;
	unsigned short m_thread_count;
	std::mutex m_connect_mutex;
	std::string m_cookie_name;
	SessionMap m_sessions;
}; // class App

}; // namespace koohar

#endif // koohar_app_hh
