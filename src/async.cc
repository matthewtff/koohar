#include "async.hh"

#ifdef _WIN32

#else /* _WIN32 */

#include <errno.h>
#include <cstring>

#endif /* _WIN32 */

#ifdef _DEBUG
#include <iostream>
#endif /* _DEBUG */

namespace koohar {

Async::Async(size_t ThreadNumber) : m_thread_number(ThreadNumber)
{
#ifdef _WIN32

	m_async = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, m_thread_number);

#else /* _WIN32 */

	m_async = epoll_create (1);

#endif /* _WIN32 */
}

void Async::append(SocketHandle SH, const bool In)
{
	m_mutex.lock();
#ifdef _WIN32

	CreateIoCompletionPort((HANDLE)SH, m_async, SH, m_thread_number + 1);
	PostQueuedCompletionStatus(m_async, 0, SH, NULL);

#else /* _WIN32 */

	struct epoll_event ev;
	ev.events = (In ? EPOLLIN : EPOLLOUT) | EPOLLET | EPOLLRDHUP;
	ev.data.fd = SH;
	if (epoll_ctl(m_async, EPOLL_CTL_ADD, SH, &ev) == -1) {
#ifdef _DEBUG
		std::cerr << "Error adding new socket to epoll\n";
		std::cerr << strerror(errno) << std::endl;
#endif /* _DEBUG */
	}

#endif /* _WIN32 */
	m_mutex.unlock();
}

AsyncKey Async::get (const int TimeOut)
{
	AsyncKey key = 0;
#ifdef _WIN32

	DWORD* transfered = new DWORD;
	LPOVERLAPPED ovrr;
	GetQueuedCompletionStatus(m_async, transfered, &key, &ovrr, TimeOut == -1 ? INFINITE : TimeOut);
	delete transfered;

#else /* _WIN32 */

	struct epoll_event ev;
	if (epoll_wait(m_async, &ev, 1, TimeOut) == 0)
		return 0;
	key = ev.data.fd;
	m_mutex.lock();
	if (epoll_ctl(m_async, EPOLL_CTL_DEL, key, &ev) == -1) {
#ifdef _DEBUG
		std::cerr << "Error removing epoll!\n"; // TODO: throw smth ... or not???
		std::cerr << strerror(errno) << std::endl;
#endif /* _DEBUG */
	}
	m_mutex.unlock();

#endif /* _WIN32 */
	return key;
}

}; // namespace koohar
