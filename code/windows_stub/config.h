// config.h



#ifndef _CONFIG_H
#define _CONFIG_H

#include <stdio.h>

#ifndef BYTE_ORDER
#include "SDL_endian.h"
#endif

#ifndef BYTE_ORDER
#define LITTLE_ENDIAN 1234
#define BIG_ENDIAN    4321

#if SDL_BYTEORDER == SDL_LIL_ENDIAN
#define BYTE_ORDER   LITTLE_ENDIAN
#elif SDL_BYTEORDER == SDL_BIG_ENDIAN
#define BYTE_ORDER   BIG_ENDIAN
#else
#error unknown byte order
#endif
#endif  // BYTE_ORDER

#if defined _WIN32

#include <direct.h> // for _mkdir, ...

#ifndef snprintf
#define snprintf _snprintf
#endif

#ifndef filelength
#define filelength _filelength
#endif

#define STUB_FUNCTION nprintf(( "Warning", "STUB: %s in " __FILE__ " at line %d\n", __FUNCTION__, __LINE__))

#else  // ! Win32


#include "SDL.h"
#include "SDL_thread.h"

#include <unistd.h>
#include <cmath>
#include <cstdint>


// don't verbose stub funtions unless we're debugging
#define STUB_FUNCTION nprintf(( "Warning", "STUB: %s in " __FILE__ " at line %d, thread %d\n", __FUNCTION__, __LINE__, getpid() ))
#define DEBUGME(d1) nprintf(( "Warning", "DEBUGME: %s in " __FILE__ " at line %d, msg \"%s\", thread %d\n", __FUNCTION__, __LINE__, d1, getpid() ))


// Standard data types
typedef std::int32_t BOOL;
typedef std::uint16_t WORD;
typedef std::uint32_t UINT;
typedef std::uint32_t DWORD;
typedef std::uint32_t FOURCC;
typedef std::uint32_t *PDWORD, *LPDWORD;

typedef struct _LARGE_INTEGER {
	std::int64_t QuadPart;
} LARGE_INTEGER;

// networking/socket stuff
#define SOCKET			int
#define SOCKADDR		struct sockaddr
#define SOCKADDR_IN		struct sockaddr_in
#define LPSOCKADDR		struct sockaddr*
#define HOSTENT			struct hostent
#define SERVENT			struct servent
#define closesocket(x)	close(x)
#define WSAEALREADY     EALREADY
#define WSAEINVAL       EINVAL
#define WSAEWOULDBLOCK  EINPROGRESS
#define WSAEISCONN      EISCONN
#define WSAENOTSOCK     ENOTSOCK
#define WSAECONNRESET   ECONNRESET
#define WSAECONNABORTED ECONNABORTED
#define WSAESHUTDOWN    ESHUTDOWN
#define WSAEADDRINUSE	EADDRINUSE
#define SOCKET_ERROR	(-1)
#define ioctlsocket(x, y, z)	ioctl(x, y, z)

#ifndef INVALID_SOCKET
#define INVALID_SOCKET ((SOCKET) -1)
#endif

// sound defines/structs
#define WAVE_FORMAT_PCM		1
#define WAVE_FORMAT_ADPCM		2
#define WAVE_FORMAT_IEEE_FLOAT	3

#pragma pack(1) // required to get proper values in ds_parse_wave()
typedef struct {
	WORD wFormatTag;
	WORD nChannels;
	DWORD nSamplesPerSec;
	DWORD nAvgBytesPerSec;
	WORD nBlockAlign;
} WAVEFORMAT;

typedef struct {
	WAVEFORMAT wf;
	WORD wBitsPerSample;
} PCMWAVEFORMAT;

typedef struct {
	WORD  wFormatTag;
	WORD  nChannels;
	DWORD nSamplesPerSec;
	DWORD nAvgBytesPerSec;
	WORD  nBlockAlign;
	WORD  wBitsPerSample;
	WORD  cbSize;
} WAVEFORMATEX;
#pragma pack()

// file related items
#define _MAX_FNAME					255
#define _MAX_PATH					255
#define MAX_PATH					255
#define _unlink(s)					unlink(s)

int filelength(int fd);
int _chdir(const char *path);
int _getcwd(char *buffer, unsigned int len);
int _mkdir(const char *path);
void _splitpath(char *path, char *drive, char *dir, char *fname, char *ext);

// string related
#define stricmp(s1, s2)			strcasecmp((s1), (s2))
#define strnicmp(s1, s2, n)		strncasecmp((s1), (s2), (n))
#define _strnicmp(s1, s2, n)	strncasecmp((s1), (s2), (n))
#define _strlwr(s)				strlwr(s)

void strlwr(char *s);
char *strnset( char *string, int fill, size_t count);

// other stuff
#define _isnan(f)     std::isnan(f)
#define _hypot(x, y)  hypot(x, y)

int MulDiv(int number, int numerator, int denominator);

struct POINT {
	int x, y;
};

#endif  // if !defined (WINDOWS)

#endif // ifndef _CONFIG_H
