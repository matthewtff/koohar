#include "base/filemapping.hh"

#include <sys/mman.h>

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
  address_ = static_cast<char*>(
      mmap(0, size_, PROT_READ, MAP_PRIVATE, file_, offset));
	if (address_ == MAP_FAILED) {
		return 0;
  }
	mapped_ = true;
	return address_ + align_size;
}

void FileMapping::UnMap() {
	munmap(static_cast<void*>(address_), size_);
	mapped_ = false;
}

} // namespace koohar
