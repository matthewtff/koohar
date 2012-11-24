#ifdef _WIN32

#else /* _WIN32 */

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>

#endif /* _WIN32 */

#include "file.hh"

#ifdef _DEBUG
#include <iostream>
#endif /* _DEBUG */

namespace koohar {

File::File () : m_size(0), m_time(0), m_opened(false)
{
	//createTemp();
}

File::File (File::Handle Hndl) : m_file(Hndl),
	m_size(0), m_time(0), m_opened(true)
{
}

File::File (const std::string& FileName) : m_fname(FileName), m_size(0),
	m_time(0), m_opened(false)
{
}

File::~File ()
{
	if(m_opened)
		close();
}

bool File::open (AccessType Mode)
{
	if (m_opened)
		return true;
#ifdef _WIN32

	if ((m_file = CreateFileA(m_fname.c_str(), Mode, FILE_SHARE_READ, NULL,
		OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE) {

#else /* _WIN32 */

	if ((m_file = ::open(m_fname.c_str(), Mode)) == -1) {

#endif /* _WIN32 */

#ifdef _DEBUG
		std::cout << "Error opening file " << m_fname << std::endl;
		std::cout << strerror(errno) << std::endl;
#endif /* _DEBUG */
		return false;
	}
	m_opened = true;
	getInfo();
	return true;
}

void File::close ()
{
	if (!m_opened)
		return;
#ifdef _WIN32

	CloseHandle(m_file);

#else /* _WIN32 */

	::close (m_file);

#endif /* _WIN32 */
	m_size = 0;
	m_time = 0;
	m_opened = false;
}

void File::remove ()
{
	if (m_opened)
		return;
#ifdef _WIN32

	if (!DeleteFileA(m_fname.c_str())) {

#else /* _WIN32 */

	if (unlink(m_fname.c_str()) == -1) {

#endif /* _WIN32 */

#ifdef _DEBUG
		std::cout << "Unable to remove file " << m_fname << std::endl;
#endif /* _DEBUG */
	}
}

bool File::move (const std::string& NewFileName)
{
	if (m_opened)
		return false;
#ifdef _WIN32

	if (!MoveFileA(m_fname.c_str(), NewFileName.c_str())) {

#else /* _WIN32 */

	if (rename(m_fname.c_str(), NewFileName.c_str()) == -1) {

#endif /* _WIN32 */

#ifdef _DEBUG
		std::cout << "Unable move file " << m_fname << std::endl;
#endif /* _DEBUG */
		return false;
	} else {
		m_fname.assign(NewFileName);
		return true;
	}
}

int File::read (void* Buffer, size_t Length)
{
	if (!m_opened)
		return IOError;
	
	return File::read(m_file, Buffer, Length);
}

int File::write (const void* Buffer, size_t Length)
{
	if (!m_opened)
		return IOError;
	
	return File::write(m_file, Buffer, Length);
}

int File::read (File::Handle Hndl, void* Buffer, size_t Length)
{
#ifdef _WIN32

	DWORD return_value = 0;
	if (!ReadFile(Hndl, Buffer, static_cast<DWORD>(Length), &return_value, NULL)) {

#else /* _WIN32 */

	int return_value = 0;
	return_value = ::read(Hndl, Buffer, Length);
	if (return_value == -1) {

#endif /* _WIN32 */

		return IOError;
	} else {
		return static_cast<int>(return_value);
	}
}


int File::write (File::Handle Hndl, const void* Buffer, size_t Length)
{
#ifdef _WIN32

	DWORD return_value = 0;
	if (!WriteFile(Hndl, Buffer, static_cast<DWORD>(Length), &return_value, NULL)) {

#else /* _WIN32 */

	int return_value = 0;
	return_value = ::write(Hndl, Buffer, Length);
	if (return_value == -1) {

#endif /* _WIN32 */

#ifdef _DEBUG
		std::cout << "Error writing to file." << std::endl;
#endif /* _DEBUG */
		return IOError;
	} else {
		return return_value;
	}
}

// private methods

void File::getInfo ()
{
#ifdef _WIN32

	m_size = static_cast<size_t>(GetFileSize(m_file, NULL));
	FILETIME write_time;
	if (GetFileTime(m_file, NULL, NULL, &write_time)) {
	   ULARGE_INTEGER ull;
	   ull.LowPart = write_time.dwLowDateTime;
	   ull.HighPart = write_time.dwHighDateTime;

	   m_time =  ull.QuadPart / 10000000ULL - 11644473600ULL;
	}

#else /* _WIN32 */

	struct stat file_info;
	fstat(m_file, &file_info);
	m_size = file_info.st_size;
	m_time = file_info.st_mtime;

#endif /* _WIN32 */
}

void File::createTemp ()
{
#ifdef _WIN32

	//TODO: For now it isn't needed, but...

#else /* _WIN32 */

	char tmp_name[9] = "./XXXXXX";
	m_file = mkstemp(tmp_name);
	m_fname = tmp_name;

#endif /* _WIN32 */
	m_opened = true;
}

}; // namespace koohar
