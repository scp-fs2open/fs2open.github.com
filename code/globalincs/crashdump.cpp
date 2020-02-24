#include "crashdump.h"

#ifdef WIN32

#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable:4091) // a microsoft header has warnings. Very nice.
#endif

// clang-format off
#include <windows.h>
#include <dbghelp.h>
// clang-format on

#ifdef _MSC_VER
#pragma warning (pop)
#endif

#include "globalincs/pstypes.h"

void make_minidump(EXCEPTION_POINTERS* e)
{
	const auto hDbgHelp = LoadLibraryA("dbghelp");
	if (hDbgHelp == nullptr)
		return;
	const auto pMiniDumpWriteDump = reinterpret_cast<decltype(&MiniDumpWriteDump)>(
	    reinterpret_cast<void*>(GetProcAddress(hDbgHelp, "MiniDumpWriteDump")));
	if (pMiniDumpWriteDump == nullptr)
		return;

	char name[MAX_PATH];
	{
		const auto nameEnd = name + GetModuleFileNameA(GetModuleHandleA(nullptr), name, MAX_PATH);
		SYSTEMTIME t;
		GetSystemTime(&t);
		wsprintfA(nameEnd - strlen(".exe"), "_%4d%02d%02d_%02d%02d%02d.mdmp", t.wYear, t.wMonth, t.wDay, t.wHour,
		          t.wMinute, t.wSecond);
	}

	const auto hFile = CreateFileA(name, GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	if (hFile == INVALID_HANDLE_VALUE)
		return;

	MINIDUMP_EXCEPTION_INFORMATION exceptionInfo;
	exceptionInfo.ThreadId          = GetCurrentThreadId();
	exceptionInfo.ExceptionPointers = e;
	exceptionInfo.ClientPointers    = FALSE;

	pMiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile,
	                   MINIDUMP_TYPE(MiniDumpWithIndirectlyReferencedMemory | MiniDumpScanMemory),
	                   e ? &exceptionInfo : nullptr, nullptr, nullptr);

	CloseHandle(hFile);

	mprintf(("CRASH DETECTED!!! Wrote crash dump to %s\n", name));
}

LONG CALLBACK unhandled_handler(EXCEPTION_POINTERS* e)
{
	make_minidump(e);
	return EXCEPTION_CONTINUE_SEARCH;
}

#endif

namespace crashdump {

void installCrashHandler()
{
#ifdef WIN32
	SetUnhandledExceptionFilter(unhandled_handler);
#endif
}

} // namespace crashdump
