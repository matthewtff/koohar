#include "filemapping.hh"

#ifdef _WIN32

#include <windows.h>

#else /* _WIN32 */

#include <sys/mman.h>

#endif /* _WIN32 */

namespace koohar {

FileMapping::FileMapping (const File::Handle& FH, size_t PageSize) :
	m_file(FH), m_page_size(PageSize), m_address(0), m_mapped(false)
{}

FileMapping::~FileMapping ()
{
	if (m_mapped)
		unMap();
}

char* FileMapping::map (const size_t Size, const size_t Offset)
{
	size_t align_size = Offset;
	size_t offset = Offset;
	/*size_t offset = m_page_size * (Offset / m_page_size);
	align_size -= offset;*/
	m_size = Size + align_size;
#ifdef _WIN32

	if ((m_file_map = CreateFileMapping(m_file, NULL, PAGE_READONLY, 0, m_size, NULL)) != NULL)
		if ((m_address = static_cast<char*>(MapViewOfFile(m_file_map, FILE_MAP_READ, 0, offset, m_size))) == NULL) {
			CloseHandle(m_file_map);
			return 0;
		}

#else /* _WIN32 */

	if ((m_address = static_cast<char*>(mmap(0, m_size, PROT_READ, MAP_PRIVATE, m_file, offset))) == MAP_FAILED)
		return 0;

#endif /* _WIN32 */
	m_mapped = true;
	return m_address + align_size;
}

void FileMapping::unMap ()
{
#ifdef _WIN32

	UnmapViewOfFile(static_cast<LPVOID>(m_address));
	CloseHandle(m_file_map);

#else /* _WIN32 */

	munmap(static_cast<void*>(m_address), m_size);

#endif /* _WIN32 */
}

}; // namespace koohar
