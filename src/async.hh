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

class Async {
public:

#ifdef _WIN32

	typedef HANDLE Handle;
	typedef DWORD Key;

#else /* _WIN32 */

	typedef int Handle;
	typedef int Key;

#endif /* _WIN32 */

	enum Type {
		Input,
		Output
	};

public:
	Async (const size_t ThreadNumber = 1);
	void append (const Socket::Handle SH, Type In = Input);
	Key get (const int TimeOut = -1);

private:
	Handle m_async;
	size_t m_thread_number;
	std::mutex m_mutex;
}; // class Async

}; // namespace koohar

#endif // koohar_async_hh
