#ifndef koohar_filemapping_hh
#define koohar_filemapping_hh

#include "file.hh"

namespace koohar {

class FileMapping {
public:
  explicit FileMapping (const File::Handle& FH);
  ~FileMapping ();
  char* map (const size_t Size, const size_t offset);
  void unMap ();

private:
  File::Handle m_file;
  size_t m_size;
  char* m_address;
  bool m_mapped;

#ifdef _WIN32

  HANDLE m_file_map;

#endif // _WIN32
}; // class FileMapping

}; // namespace koohar

#endif // koohar_filemapping_hh
