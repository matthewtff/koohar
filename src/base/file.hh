#ifndef koohar_file_hh
#define koohar_file_hh

#include <string>

#ifdef _WIN32

#include <winsock2.h>

#else /* _WIN32 */

#include <fcntl.h>

#endif /* _WIN32 */

namespace koohar {

class File {
public:

#ifdef _WIN32

  enum class AccessType : DWORD {
    ReadOnly = GENERIC_READ;
    WriteOnly = GENERIC_WRITE;
    ReadWrite = (GENERIC_READ | GENERIC_WRITE);
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

public:
  File ();
  explicit File (File::Handle Hndl);
  // This constructor just assigns filename, but file is NOT BEING OPENED.
  explicit File (const std::string& FileName);
  ~File ();
  bool open (AccessType Mode);
  void close ();
  void remove ();
  bool move (const std::string& NewFileName);
  int read (void* Buffer, const size_t Length) const;
  int write (const void* Buffer, const size_t Length);
  std::string readToString() const;
  size_t getSize () const { return m_size; }
  time_t getTime () const { return m_time; }
  Handle getHandle () const { return m_file; }

public:
  static int read (Handle Hndl, void* Buffer, const size_t Length);
  static int write (Handle Hndl, const void* Buffer, const size_t Length);

  static bool isDirectory (const char* Path);

private:
  void getInfo ();
  void createTemp ();

private:
  Handle m_file;
  std::string m_fname;
  size_t m_size;
  time_t m_time;
  bool m_opened;

}; // class File

} // namespace koohar

#endif // koohar_file_hh