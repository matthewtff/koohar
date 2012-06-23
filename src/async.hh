#ifndef koohar_async_hh
#define koohar_async_hh

#ifdef _WIN32

#include <winsock2.h>
#include <boost/signals2/mutex.hpp>

namespace std {
	using boost::signals2::mutex;
}

#else /* _WIN32 */

#include <sys/epoll.h>
#include <mutex>

#endif /* _WIN32 */

#include "socket.hh"

namespace koohar {

#ifdef _WIN32

#define AsyncHandle HANDLE
#define AsyncKey DWORD

#else /* _WIN32 */

#define AsyncHandle int
#define AsyncKey int

#endif /* _WIN32 */

class Async {
public:
	Async (const size_t ThreadNumber = 1);
	void append (const SocketHandle SH, const bool In = true);
	AsyncKey get (const int TimeOut = -1);
private:
	AsyncHandle m_async;
	size_t m_thread_number;
	std::mutex m_mutex;
}; // class Async

}; // namespace koohar

#endif // koohar_async_hh
