#include "base/file.hh"

#include <cassert>
#include <vector>

#include "base/utils.hh"

namespace koohar {

File::File() : size_(0), time_(0), opened_(false) {}

File::File(Handle handle) : file_(handle), size_(0), time_(0), opened_(true) {}

File::File(const std::string& file_name)
    : fname_(file_name), size_(0), time_(0), opened_(false) {
}

File::~File() {
  if (opened_) {
    Close();
  }
}

bool File::Open(AccessType Mode) {
  if (opened_) {
    return true;
  }
  file_ = CreateFileA(fname_.c_str(), static_cast<DWORD>(Mode),
                      FILE_SHARE_READ, NULL, OPEN_ALWAYS,
                      FILE_ATTRIBUTE_NORMAL, NULL);
  if (file_ == INVALID_HANDLE_VALUE) {
    LOG << "Error opening file " << fname_ << std::endl;
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
  CloseHandle(file_);
  size_ = 0;
  time_ = 0;
  opened_ = false;
}

void File::Remove() {
  if (opened_) {
    return;
  }
  if (!DeleteFileA(fname_.c_str())) {
    LOG << "Unable to remove file " << fname_ << std::endl;
  }
}

bool File::Move(const std::string& new_file_name) {
  if (opened_) {
    return false;
  }
  if (!MoveFileA(fname_.c_str(), new_file_name.c_str())) {
    LOG << "Unable move file " << fname_ << std::endl;
    return false;
  }
  fname_.assign(new_file_name);
  return true;
}

int File::Read(void* buffer, const size_t length) const {
  assert(opened_);
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
  DWORD return_value = 0;
  if (!ReadFile(handle,
                buffer,
                static_cast<DWORD>(length),
                &return_value,
                NULL)) {
    return IOError;
  }
  return static_cast<int>(return_value);
}

int File::Write(Handle handle, const void* buffer, const size_t length) {
  DWORD return_value = 0;
  if (!WriteFile(handle,
                 buffer,
                 static_cast<DWORD>(length),
                 &return_value,
                 NULL)) {
    LOG << "Error writing to file." << std::endl;
    return IOError;
  }
  return static_cast<int>(return_value);
}

bool File::IsDirectory(const char* path) {
  DWORD file_attributes = GetFileAttributes(path);
  if (file_attributes != INVALID_FILE_ATTRIBUTES) {
    return !!(file_attributes & FILE_ATTRIBUTE_DIRECTORY);
  }
  return false;
}

// private methods

void File::GetInfo() {
  size_ = static_cast<size_t>(GetFileSize(file_, NULL));
  FILETIME write_time;
  if (GetFileTime(file_, NULL, NULL, &write_time)) {
     ULARGE_INTEGER ull;
     ull.LowPart = write_time.dwLowDateTime;
     ull.HighPart = write_time.dwHighDateTime;

     time_ =  ull.QuadPart / 10000000ULL - 11644473600ULL;
  }
}

void File::CreateTemp() {
  //TODO(matthewtff): Implement.
  return;
}

}  // namespace koohar
