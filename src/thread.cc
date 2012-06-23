#include "thread.hh"

namespace koohar {

void createThread (ThreadHandle& TH, ThreadFunc(Func), ThreadGetValue GetValue)
{
#ifdef _WIN32

	TH = CreateThread(NULL, 0, Func, GetValue, 0, NULL);

#else /* _WIN32 */

	pthread_create(&TH, NULL, Func, GetValue);

#endif /* _WIN32 */
}

void joinThread (ThreadHandle& TH)
{
#ifdef _WIN32

	WaitForSingleObject(TH, INFINITE);

#else /* _WIN32 */

	pthread_join(TH, NULL);

#endif /* _WIN32 */	
}

}; // namespace koohar
