#include "base/file.hh"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

#include "base/utils.hh"

namespace koohar {

File::File() : file_(0), size_(0), time_(0), opened_(false) {}

File::File(Handle handle) : file_(handle), size_(0), time_(0), opened_(true) {}

File::File(const std::string& file_name)
    : fname_(file_name), size_(0), time_(0), opened_(false)
{}

File::~File() {
  if (opened_) {
    Close();
  }
}

bool File::Open(const AccessType Mode) {
  if (opened_) {
    return true;
  }
  file_ = open(fname_.c_str(), static_cast<int>(Mode));
  if (file_ == -1) {
    LOG(kError) << "Error opening file '" << fname_ << "': " <<
        strerror(errno) << std::endl;
    return false;
  }
  opened_ = true;
  GetInfo();
  return true;
}

void File::Close() {
  if (!opened_) {
    return;
  }
  close (file_);
  size_ = 0;
  time_ = 0;
  opened_ = false;
}

void File::Remove() {
  if (opened_) {
    return;
  }
  if (unlink(fname_.c_str()) == -1) {
    LOG(kError) << "Unable to remove file '" << fname_ << "'" << std::endl;
  }
}

bool File::Move(const std::string& new_file_name) {
  if (opened_) {
    return false;
  }
  if (rename(fname_.c_str(), new_file_name.c_str()) == -1) {
    LOG(kError) << "Unable move file '" << fname_ << "'" << std::endl;
    return false;
  }
  fname_ = new_file_name;
  return true;
}

int File::Read(void* buffer, const size_t length) const {
  if (!opened_) {
    return IOError;
  }
  return Read(file_, buffer, length);
}

int File::Write(const void* buffer, const size_t length) {
  if (!opened_) {
    return IOError;
  }
  return Write(file_, buffer, length);
}

std::string File::ReadToString() const {
  if (!opened_) {
    return std::string();
  }
  std::vector<char> file_data(size_);
  const int readed = Read(reinterpret_cast<void*>(&file_data[0]), size_);
  if (readed < 0 || readed > static_cast<int>(size_)) {
    return std::string();
  }
  return std::string(file_data.begin(), file_data.end());
}

int File::Read(Handle handle, void* buffer, const size_t length) {
  const int return_value = read(handle, buffer, length);
  if (return_value == -1) {
    LOG(kError) << "Error reading from file: " << strerror(errno) << std::endl;
  }
  return static_cast<int>(return_value);
}

int File::Write(Handle handle, const void* buffer, const size_t length) {
  const int return_value = write(handle, buffer, length);
  if (return_value == -1) {
    LOG(kError) << "Error writing to file: " << strerror(errno) << std::endl;
    return IOError;
  }
  return static_cast<int>(return_value);
}

bool File::IsDirectory(const char* path) {
  struct stat info;
  if (stat(path, &info) == 0) {
    return static_cast<bool>(info.st_mode & S_IFDIR);
  }
  return false;
}

// private methods

void File::GetInfo() {
  struct stat file_info;
  fstat(file_, &file_info);
  size_ = file_info.st_size;
  time_ = file_info.st_mtime;
}

void File::CreateTemp() {
  char tmp_name[] = "./XXXXXX";
  file_ = mkstemp(tmp_name);
  fname_ = tmp_name;
  opened_ = true;
}

} // namespace koohar
