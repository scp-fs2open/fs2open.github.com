// config.h

/*
 * $Logfile: $
 * $Revision: 2.4 $
 * $Date: 2004-08-11 05:06:36 $
 * $Author: Kazan $
 *
 * OS-dependent definitions.
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.3  2003/03/02 06:25:31  penguin
 * Tweaked for gcc
 *  - penguin
 *
 * Revision 2.2  2002/07/22 01:06:04  penguin
 * More defines for winsock compatibility
 *
 * Revision 2.1  2002/07/07 19:56:00  penguin
 * Back-port to MSVC
 *
 * $NoKeywords: $
 */

#include "PreProcDefines.h"
#ifndef _CONFIG_H
#define _CONFIG_H


#if defined _WIN32

#if !defined BYTE_ORDER
 #define LITTLE_ENDIAN 1234
 #define BIG_ENDIAN    4321

 #if defined _M_IX86 || defined _X86_
  #define BYTE_ORDER   LITTLE_ENDIAN
 #else
  #error unknown byte order
 #endif
#endif  // BYTE_ORDER


#else  // ! Win32

#include <limits.h>
#include <unistd.h>
#include <pthread.h>


#define _cdecl
#define __cdecl
#define __stdcall
#define PASCAL


#ifndef _MAX_PATH
#define _MAX_PATH PATH_MAX
#endif

#ifndef MAX_PATH
#define MAX_PATH PATH_MAX
#endif

#ifndef MAX_PATH_LEN
#define MAX_PATH_LEN PATH_MAX
#endif

#ifndef MAX_FILENAME_LEN
#define MAX_FILENAME_LEN NAME_MAX
#endif

#ifndef _MAX_FNAME
#define _MAX_FNAME NAME_MAX
#endif




// TODO -- make portable
typedef unsigned char ubyte;
typedef unsigned char byte;
typedef unsigned int  DWORD;
typedef bool          BOOL;
typedef unsigned int  UINT;
typedef unsigned short WORD;

#define Sleep(t)  os_sleep(t)

struct _EXCEPTION_POINTERS;


// these a wrong...
struct STARTUPINFO {
	int cb;
};
struct PROCESS_INFORMATION { };
struct MEMORYSTATUS { 
	int dwLength;
	int dwTotalPhys;
	int dwTotalVirtual;
};
struct WIN32_FIND_DATA { 
	int dwFileAttributes;
	char * cFileName;
	int nFileSizeLow;
};
typedef struct {
	int attrib;
	char *name;
	int time_write;
	unsigned int size;
} _finddata_t;

struct POINT {
	int x, y;
};


typedef void * HWND;
typedef void * HINSTANCE;
typedef void * HANDLE;

typedef char * LPSTR;

#define MB_OK 0
#define MB_TASKMODAL 0
#define MB_SETFOREGROUND 0
#define MB_OKCANCEL 0
#define IDCANCEL 0

#define CREATE_DEFAULT_ERROR_MODE 0

#define FILE_ATTRIBUTE_NORMAL 0
#define FILE_ATTRIBUTE_DIRECTORY 0

#define INVALID_HANDLE_VALUE ((HANDLE) NULL)

#define DRIVE_CDROM 0

#define _A_SUBDIR 0
#define _A_RDONLY 0

#define GENERIC_READ 0
#define FILE_SHARE_READ 0
#define OPEN_EXISTING 0
#define PAGE_READONLY 0
#define FILE_MAP_READ 0


#ifdef unix
// -- socket stuff
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
typedef int SOCKET;
typedef sockaddr SOCKADDR;
typedef sockaddr_in SOCKADDR_IN;
typedef SOCKADDR *LPSOCKADDR;
typedef hostent HOSTENT;
typedef servent SERVENT;

#ifndef INVALID_SOCKET
#define INVALID_SOCKET ((SOCKET) -1)
#endif

#define SOCKET_ERROR (-1)

#ifdef unix
typedef pthread_mutex_t CRITICAL_SECTION;
#else
typedef void * CRITICAL_SECTION;
#endif


//  struct LARGE_INTEGER {
//  	unsigned int LowPart, HighPart, QuadPart;
//  };

// time & select
#include <sys/time.h>
typedef timeval TIMEVAL;

// ioctl stuff
#include <sys/ioctl.h>
#endif // ifdef unix


// just to keep the compiler from complaining...
extern "C" {

	#define stricmp(s1, s2)       strcasecmp((s1), (s2))
	#define strnicmp(s1, s2, n)   strncasecmp((s1), (s2), (n))
	#define _strnicmp(s1, s2, n)  strncasecmp((s1), (s2), (n))

	void strlwr(char *);

	#define _isnan(f)     isnan(f)
	#define _hypot(x, y)  hypot(x, y)

	int MulDiv(int number, int numerator, int denominator);

//  	DWORD GetFileAttributes(const char *);
//  	void SetFileAttributes(const char *, int);
//  	int CopyFile(const char *, const char *, int);
//  	void DeleteFile(const char *);
//  	void RemoveDirectory(const char *);

//  	HANDLE FindFirstFile(char *, WIN32_FIND_DATA *);
//  	int FindNextFile(HANDLE, WIN32_FIND_DATA *);
//  	void FindClose(HANDLE);

//  	int _findfirst(const char *, _finddata_t *);
//  	int _findnext(int, _finddata_t *);
//  	void _findclose(int);



//  	int _getdrive();
//  	void _chdrive(int);

	int filelength(int);

	int _chdir(const char *);
	#define SetCurrentDirectory(s) _chdir(s)

	int _getcwd(char *, unsigned int);
	#define GetCurrentDirectory(i, s) _getcwd((s), (i))

	int _mkdir(const char *);


	#define _strlwr(s)				 strlwr(s)

//  	int GetDriveType(const char *);
//  	int GetVolumeInformation(const char *, const char *, int, void *, void *, void *, void *, int);

	void _splitpath(char *, char *, char *, char *, char *);

	#define _unlink(s) unlink(s)

	char *itoa(int, char *, int);


//  	BOOL CreateProcess(const char *,
//  							 const char *,
//  							 void *,
//  							 void *,
//  							 BOOL,
//  							 unsigned int,
//  							 void *,
//  							 const char *,
//  							 STARTUPINFO *,
//  							 PROCESS_INFORMATION *);

//  	int MessageBox(HWND, const char *, const char *, int);
//  	HWND FindWindow(const char *, void * );
//  	void SetForegroundWindow(HWND);

//  	int _access(const char *, int);

//  	void timeBeginPeriod(int);
//  	void timeEndPeriod(int);

//  	void GlobalMemoryStatus(MEMORYSTATUS *);

//  	int GetModuleFileName(HINSTANCE, const char *, int);

//  	_EXCEPTION_POINTERS * GetExceptionInformation();

//  	HANDLE CreateFile(const char *, int, int, void *, int, int, void *);
//  	HANDLE CreateFileMapping(HANDLE, void *, int, int, int, void *);
//  	ubyte * MapViewOfFile(HANDLE, int, int, int, int);
//  	int UnmapViewOfFile(void *);
//  	int CloseHandle(HANDLE);


//  	void * _beginthread(void(*)(void *), int, void *);

//  	void closesocket(SOCKET);

//  	void OutputDebugString(const char *);

//  	unsigned int inet_addr(const char *);
//  	int WSAGetLastError();

#define WSAEALREADY     EALREADY
#define WSAEINVAL       EINVAL
#define WSAEWOULDBLOCK  EWOULDBLOCK
#define WSAEISCONN      EISCONN
#define WSAENOTSOCK     ENOTSOCK
#define WSAECONNRESET   ECONNRESET
#define WSAECONNABORTED ECONNABORTED
#define WSAESHUTDOWN    ESHUTDOWN

	void DeleteCriticalSection(CRITICAL_SECTION *);
	void InitializeCriticalSection(CRITICAL_SECTION *);
	void EnterCriticalSection(CRITICAL_SECTION *);
	void LeaveCriticalSection(CRITICAL_SECTION *);
}


#endif  // if !defined (WINDOWS)

#endif // ifndef _CONFIG_H
