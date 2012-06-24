#ifndef koohar_win_hh
#define koohar_win_hh

#ifdef _WIN32

#include <winsock2.h>

#define HandlerRet BOOL WINAPI
typedef DWORD HandlerGet;

void setHandler(int Add, PHANDLER_ROUTINE Routine)
{
	SetConsoleCtrlHandler(Routine, Add);
}

#else /* _WIN32 */

#include <signal.h>

typedef void HandlerRet;
typedef int HandlerGet;

enum {
	CTRL_C_EVENT = SIGINT
};

void setHandler(int Sig, HandlerRet(* Routine)(HandlerGet))
{
	signal(Sig, Routine);
}

#endif /* _WIN32 */

#endif // koohar_win_hh
