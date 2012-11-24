#ifndef koohar_thread_hh
#define koohar_thread_hh

#ifdef _WIN32

#include <winsock2.h>

#else /* _WIN32 */

#include <pthread.h>

#endif /* _WIN32 */

namespace koohar {

#ifdef _WIN32

#define ThreadReturnValue DWORD WINAPI
#define ThreadGetValue LPVOID
#define ThreadHandle HANDLE
#define ThreadFunc(x) LPTHREAD_START_ROUTINE x

#else /* _WIN32 */

typedef void* ThreadReturnValue;
typedef void* ThreadGetValue;
typedef pthread_t ThreadHandle;
typedef ThreadReturnValue (*ThreadFunc)(ThreadGetValue);

#endif /* _WIN32 */

void createThread (ThreadHandle& TH, ThreadFunc(Func), ThreadGetValue GetValue);
void joinThread (ThreadHandle& TH);

}; // namespace koohar

#endif // koohar_thread_hh
