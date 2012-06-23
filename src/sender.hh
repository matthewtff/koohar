#ifndef koohar_sender_hh
#define koohar_sender_hh

#include <cstdlib>
#include <map>

#ifdef _WIN32

#include <boost/signals2/mutex.hpp>

namespace std {
	using boost::signals2::mutex;
}

#else /* _WIN32 */

#include <mutex>

#endif /* _WIN32 */

#include "filemapping.hh"
#include "thread.hh"
#include "socket.hh"
#include "async.hh"

namespace koohar {

class Response;
class Sender;

struct SendData {

	enum DataType {
		Data,
		File,
		Close // close socket after sending all data
	};
	
	SendData (char* NewData, const size_t NewSize, const DataType NewType,
		const size_t NewId, FileMapping* Map) :
		m_data(NewData), m_size(NewSize), m_offset(0), m_type(NewType),
		m_id(NewId), m_map(Map)
	{}

	char* m_data;
	size_t m_size;
	size_t m_offset;
	DataType m_type;
	size_t m_id;
	FileMapping* m_map;
	
	bool operator < (const SendData& That) const { return m_id < That.m_id; }
	
}; // struct SendData

typedef std::multimap<SocketHandle, SendData> SenderMap;

/**
 *	Represents thread for asyncronious data sending.
 */
class Sender {
public:
	
	typedef struct ThreadInfo {

		ThreadInfo (SenderMap& map, Async& async, std::mutex& map_mutex) :
			m_map(map), m_async(async), m_map_mutex(map_mutex) {}

		SenderMap& m_map;
		Async& m_async;
		std::mutex& m_map_mutex;
	} ThreadInfo; // struct ThreadInfo
	
	Sender ();
	~Sender ();
	bool send (const SocketHandle SomeSocket, const char* SomeData, const size_t SomeSize);
	bool sendFile (const SocketHandle SomeSocket, const char* SomeFileName, const size_t SomeSize, const size_t SomeOffset);
	void close (const SocketHandle SomeSocket);
private:
	SenderMap m_map;
	std::mutex m_map_mutex;
	Async m_async;
	ThreadHandle m_th;
	//size_t m_page_size;
	
	size_t countEntries (const SocketHandle SomeSocket);
}; // class Sender

}; // namespace koohar

#endif // koohar_sender_hh

