#include "sender.hh"

#include <errno.h>
#include <unistd.h>
#include <cstring>
#include <algorithm>

#include "response.hh"
#include "file.hh"

#ifdef _DEBUG
#include <iostream>
#endif /* _DEBUG */

namespace koohar {

void clearMapNode (const std::pair<const Socket::Handle, const SendData>& data)
{
	if (data.second.m_type == SendData::Data)
		delete []data.second.m_data;
	else if (data.second.m_type == SendData::File)
		delete data.second.m_map;
}

int sendData (const Socket::Handle Socket, const SendData& Data)
{
	if (!Data.m_data || !Data.m_size || !Socket)
		return -1;
	size_t sent = 0;
#ifdef _WIN32

	OVERLAPPED ovr;
	ovr.Offset = 0;
	ovr.OffsetHigh = 0;
	ovr.hEvent = NULL;
	DWORD left = Data.m_size - Data.m_offset;
	DWORD tsent = 0;
	while (left) {
		if (!WriteFile ((HANDLE)Socket, static_cast<const char*>(Data.m_data + sent + Data.m_offset), left, &tsent, &ovr)) {
			DWORD err = GetLastError();
			if (err != ERROR_HANDLE_EOF) {
#ifdef _DEBUG
				char msg[2000];
				FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), msg, 2000, NULL);
				std::cout << msg << std::endl;
#endif /* _DEBUG */
				return -1;
			}
			return sent;
		}

#else /* _WIN32 */

	size_t left = Data.m_size - Data.m_offset;
	long long tsent = 0; // Signed type needed cause of -1 return value on error
	while(left) {
		if ((tsent = write(Socket, static_cast<const char*>(Data.m_data + sent + Data.m_offset), left)) < 0) {
			if (errno != EAGAIN) {
#ifdef _DEBUG
				std::cout << strerror(errno) << std::endl;
#endif /* _DEBUG */
				return -1;
			} else
				return sent;
		}

#endif /* _WIN32 */
		sent += tsent;
		left -= tsent;
	}
	return sent;
}

ThreadReturnValue sender_thread (ThreadGetValue Info)
{
	Sender::ThreadInfo& info = *(reinterpret_cast<Sender::ThreadInfo*>(Info));
	while (true) {
		Async::Key key = info.m_async.get();
		if (!key)
			continue;
		info.m_map_mutex.lock();
		SenderMap::iterator it = info.m_map.find(key);
		if (it != info.m_map.end()) {
			while (it != info.m_map.end() && it->first == key) {
				if (it->second.m_type == SendData::File || it->second.m_type == SendData::Data) {
					int sent = sendData (it->first, it->second);
					it->second.m_offset += sent;
					if (it->second.m_offset == it->second.m_size || sent == -1) { // If we sent all data, then erase it from map (Or error accured)
						clearMapNode (*it);
						info.m_map.erase(it);
						it = info.m_map.find(key);
					} else { // Otherwise just pull socket back to epoll
						info.m_async.append(it->first, Async::Output);
						break;
					}
				} else if (it->second.m_type == SendData::Close) {
					SenderMap::iterator tit = it; ++tit;
					if (tit != info.m_map.end() && tit->first == it->first)
						++it;
					else {
						close(it->first);
						info.m_map.erase(it);
						break;
					}
				}
			}
		}
		info.m_map_mutex.unlock();
	}
	delete &info;
	return 0;
}

Sender::Sender () //: _async(2)
{
	//m_page_size = sysconf(_SC_PAGESIZE);
	createThread(m_th, sender_thread, reinterpret_cast<ThreadGetValue>(new ThreadInfo(m_map, m_async, m_map_mutex)));
}

Sender::~Sender ()
{
	joinThread(m_th);
	// We want to be sure there is no memory leak on object destroy
	std::for_each(m_map.begin(), m_map.end(), clearMapNode);
}

bool Sender::send (const Socket::Handle SomeSocket, const char* SomeData, const size_t SomeSize)
{
	if (!SomeData || !SomeSize)
		return false;
	char* PutData = new char[SomeSize];
	memcpy(PutData, SomeData, SomeSize);
	m_map_mutex.lock();
	m_map.insert(std::pair<int, SendData>(SomeSocket, SendData(PutData, SomeSize, SendData::Data, countEntries(SomeSocket), 0)));
	m_map_mutex.unlock();
	m_async.append(SomeSocket, Async::Output);
	return true;
}

bool Sender::sendFile (const Socket::Handle SomeSocket, const char* SomeFileName, const size_t SomeSize, const size_t SomeOffset)
{
	if (!SomeFileName)
		return false;
	File static_file (SomeFileName);
	if (!static_file.open(File::ReadOnly))
		return false;
	
	size_t real_size = SomeSize ? SomeSize : static_file.size();
	// try to map file : should be fastest
	FileMapping* map = new FileMapping(static_file.fh());
	char* mapped_file = map->map(real_size, SomeOffset);
	m_map_mutex.lock();
	m_map.insert(std::pair<int, SendData>(SomeSocket, SendData(mapped_file, real_size, SendData::File, countEntries(SomeSocket), map)));
	m_map_mutex.unlock();
	m_async.append(SomeSocket, Async::Output);
	return true;
}

void Sender::close (const Socket::Handle SomeSocket)
{
	m_map_mutex.lock();
	m_map.insert(std::pair<int, SendData>(SomeSocket, SendData(0, 0, SendData::Close, countEntries(SomeSocket), 0)));
	m_map_mutex.unlock();
	m_async.append(SomeSocket, Async::Output);
}

// private methods

size_t Sender::countEntries (const Socket::Handle SomeSocket)
{
	size_t ret = 0;
	for (auto it = m_map.find(SomeSocket);
		it != m_map.end() && it->first == SomeSocket; ++it)
	{
		ret = it->second.m_id;
	}
	return ret + 1;
}

}; // namespace koohar
