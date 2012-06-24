#ifndef koohar_filemapping_hh
#define koohar_filemapping_hh

#include "file.hh"

#ifdef _WIN32

#else /* _WIN32 */

#include <sys/mman.h>

#endif /* _WIN32 */

namespace koohar {

class FileMapping {
public:
	FileMapping (const FileHandle& FH, const size_t PageSize = 65536); // 64KB
	~FileMapping ();
	char* map (const size_t Size, const size_t offset);
	void unMap ();

private:
	FileHandle m_file;
	size_t m_page_size;
	size_t m_align_size; // Mapping offset should be multiple of page_size, so some unneeded data could be maped
	size_t m_size;
	char* m_address;
	bool m_mapped;

#ifdef _WIN32

	HANDLE m_file_map;

#endif // _WIN32
}; // class FileMapping

}; // namespace koohar

#endif // koohar_filemapping_hh
