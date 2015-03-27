#ifndef koohar_file_hh
#define koohar_file_hh

#ifdef _WIN32
#include <winsock2.h>
#else /* _WIN32 */
#include <fcntl.h>
#endif /* _WIN32 */

#include <string>

namespace koohar {

class File {
 public:
#ifdef _WIN32
  enum class AccessType : DWORD {
    ReadOnly = GENERIC_READ,
    WriteOnly = GENERIC_WRITE,
    ReadWrite = (GENERIC_READ | GENERIC_WRITE),
  };
  typedef HANDLE Handle;
#else /* _WIN32 */
  enum class AccessType : int {
    ReadOnly = O_RDONLY,
    WriteOnly = O_WRONLY,
    ReadWrite = O_RDWR
  };
  typedef int Handle;
#endif /* _WIN32 */

  enum Error {
    IOError = -1,
  };

  File();
  explicit File(File::Handle handle);
  // This constructor just assigns filename, but file is NOT BEING OPENED.
  explicit File(const std::string& file_name);
  ~File();
  bool Open(AccessType Mode);
  void Close();
  void Remove();
  bool Move(const std::string& new_file_name);
  int Read(void* buffer, const size_t length) const;
  int Write(const void* buffer, const size_t length);
  std::string ReadToString() const;
  size_t GetSize() const { return size_; }
  time_t GetTime() const { return time_; }
  Handle GetHandle() const { return file_; }

  static int Read(Handle handle, void* buffer, const size_t length);
  static int Write(Handle handle, const void* buffer, const size_t length);
  static bool IsDirectory(const char* path);

 private:
  void GetInfo();
  void CreateTemp();

  Handle file_;
  std::string fname_;
  size_t size_;
  time_t time_;
  bool opened_;
}; // class File

} // namespace koohar

#endif // koohar_file_hh
