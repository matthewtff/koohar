#include "base/filemapping.hh"

#include <windows.h>

namespace koohar {

FileMapping::FileMapping(const File::Handle handle)
    : file_(handle),
      address_(0),
      mapped_(false) {
}

FileMapping::~FileMapping() {
	if (mapped_) {
		UnMap();
  }
}

char* FileMapping::Map(const size_t size, const size_t offset) {
  // TODO(matthewtff): WTF? remove this offset cheating...
	const size_t align_size = offset;
	size_ = size + align_size;
  file_map_ = CreateFileMapping(file_, NULL, PAGE_READONLY, 0, size_, NULL);
	if (file_map_ != NULL) {
    address_ = static_cast<char*>(
        MapViewOfFile(file_map_, FILE_MAP_READ, 0, offset, size_));
		if (!address_) {
			CloseHandle(file_map_);
			return 0;
		}
  }
	mapped_ = true;
	return address_ + align_size;
}

void FileMapping::UnMap() {
	UnmapViewOfFile(static_cast<LPVOID>(address_));
	CloseHandle(file_map_);
	mapped_ = false;
}

} // namespace koohar
