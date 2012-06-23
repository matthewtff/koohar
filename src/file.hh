#ifndef _koohar_file_hh
#define _koohar_file_hh

#include <string>

#ifdef _WIN32

#include <winsock2.h>

#else /* _WIN32 */

#include <fcntl.h>

#endif /* _WIN32 */

namespace koohar {

#ifdef _WIN32

enum {
	READ_ONLY = GENERIC_READ,
	WRITE_ONLY = GENERIC_WRITE,
	READ_WRITE = (GENERIC_READ | GENERIC_WRITE)
};

typedef HANDLE FileHandle;

#else /* _WIN32 */

enum {
	READ_ONLY = O_RDONLY,
	WRITE_ONLY = O_WRONLY,
	READ_WRITE = O_RDWR
};

typedef int FileHandle;

#endif /* _WIN32 */

class File {
public:
	File ();
	File (const std::string& FileName);
	~File ();
	bool open (int Flags);
	void close ();
	void remove ();
	bool move (const std::string& NewFileName);
	int read (void* Buffer, size_t Length);
	int write (const void* Buffer, size_t Length);
	size_t size () const { return m_size; }
	time_t time () const { return m_time; }
	FileHandle fh () const { return m_file; }

private:
	FileHandle m_file;
	std::string m_fname;
	size_t m_size;
	time_t m_time;
	bool m_opened; // true if file is opened
	
	void getInfo ();
	void createTemp ();
}; // class File

}; // namespace koohar

#endif // _koohar_file_hh
