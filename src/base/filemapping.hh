#ifndef koohar_filemapping_hh
#define koohar_filemapping_hh

#include "base/file.hh"

namespace koohar {

class FileMapping {
 public:
  explicit FileMapping(const File::Handle handle);
  ~FileMapping();
  char* Map(const size_t Size, const size_t offset);
  void UnMap();

 private:
  const File::Handle file_;
  size_t size_;
  char* address_;
  bool mapped_;
#ifdef _WIN32
  File::Handle file_map_;
#endif // _WIN32
}; // class FileMapping

} // namespace koohar

#endif // koohar_filemapping_hh
