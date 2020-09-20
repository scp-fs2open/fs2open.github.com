


#ifndef _WIN32

#include <cctype>
#include <cerrno>
#include <fcntl.h>
#include <cstdarg>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>

#include "windows_stub/config.h"

#include "platformChecks.h"

#if defined(APPLE_APP)
#include <CoreFoundation/CoreFoundation.h>
#endif

#if defined(SCP_HAVE_MALLOC_H)
#include <malloc.h>
#endif

#ifdef SCP_HAVE_EXECINFO_H
#include <execinfo.h>
#endif

#ifdef SCP_HAVE_CXXAPI_H
#include <cxxabi.h>
#endif

#include "cmdline/cmdline.h"
#include "debugconsole/console.h"
#include "globalincs/pstypes.h"

// find the size of a file
int filelength(int fd)
{
	struct stat buf;

	if (fstat (fd, &buf) == -1)
		return -1;

	return buf.st_size;
}


SCP_string dump_stacktrace()
{
#ifdef SCP_HAVE_EXECINFO_H
	// The following is adapted from here: https://panthema.net/2008/0901-stacktrace-demangled/
	const int ADDR_SIZE = 64;
	void *addresses[ADDR_SIZE];
	
	auto numstrings = backtrace(addresses, ADDR_SIZE);
	
	if (numstrings == 0)
	{
		return "No stacktrace available (possibly corrupt)";
	}
	
	auto symbollist = backtrace_symbols(addresses, numstrings);
	
	if (symbollist == nullptr)
	{
		return "No stacktrace available (possibly corrupt)";
	}
	
	// Demangle c++ function names to a more readable format using the ABI functions
	// TODO: Maybe add configure time checks to check if the required features are available
	SCP_stringstream stackstream;
#ifdef SCP_HAVE_CXXAPI_H
	size_t funcnamesize = 256;
	char* funcname = reinterpret_cast<char*>(malloc(funcnamesize));
	
	// iterate over the returned symbol lines. skip the first, it is the
	// address of this function.
	// NOTE: the numstrings type is different on different systems,
	// so we use decltype here. See GitHub #1138.
	for (decltype(numstrings) i = 1; i < numstrings; i++)
	{
		char *begin_name = 0, *begin_offset = 0, *end_offset = 0;

		// find parentheses and +address offset surrounding the mangled name:
		// ./module(function+0x15c) [0x8048a6d]
		for (char *p = symbollist[i]; *p; ++p)
		{
			if (*p == '(')
				begin_name = p;
			else if (*p == '+')
				begin_offset = p;
			else if (*p == ')' && begin_offset) {
				end_offset = p;
				break;
			}
		}

		if (begin_name && begin_offset && end_offset && begin_name < begin_offset)
		{
			*begin_name++ = '\0';
			*begin_offset++ = '\0';
			*end_offset = '\0';

			// mangled name is now in [begin_name, begin_offset) and caller
			// offset in [begin_offset, end_offset). now apply
			// __cxa_demangle():

			int status;
			char* ret = abi::__cxa_demangle(begin_name, funcname, &funcnamesize, &status);
			
			if (status == 0) {
				funcname = ret; // use possibly realloc()-ed string
				stackstream << "  " << symbollist[i] << " : " << funcname << "+" << begin_offset << "\n";
			}
			else {
				// demangling failed. Output function name as a C function with
				// no arguments.
				stackstream << "  " << symbollist[i] << " : " << begin_name << "()+" << begin_offset << "\n";
			}
		}
		else
		{
			// couldn't parse the line? print the whole line.
			stackstream << "  " << symbollist[i] << "\n";
		}
	}
	free(funcname);
#else
	for (auto i = 0; i < numstrings; ++i) {
		stackstream << symbollist[i] << "\n";
	}
#endif

	free(symbollist);

	return stackstream.str();
#else
	return "No stacktrace available";
#endif
}

// get a filename minus any leading path
char *clean_filename(char *name)
{
	char *p = name + strlen(name)-1;

	// Move p to point to first letter of EXE filename
	while( (p > name) && (*p != '\\') && (*p != '/') && (*p != ':') )
		p--;

	p++;

	return p;
}

// retrieve the current working directory
int _getcwd(char *out_buf, unsigned int len)
{
	if (getcwd(out_buf, len) == NULL) {
		Error(__FILE__, __LINE__, "buffer overflow in getcwd (buf size = %u)", len);
	}

	return 1;
}

// change directory to specified path
int _chdir(const char *path)
{
	int status = chdir(path);

#ifndef NDEBUG
	int m_error = errno;

	if (status) {
		Warning(__FILE__, __LINE__, "Cannot chdir to %s: %s", path, strerror(m_error));
	}
#endif

	return status;
}

// make specified directory
int _mkdir(const char *path)
{
	// Windows _mkdir does not take file permissions as a parameter.
	// umask already deals with that, so 0777 should be fine.
	return mkdir(path, 0777);
}

void _splitpath (const char *path, char * /*drive*/, char *dir, char *fname, char *ext)
{
	if ( (path == NULL) || (fname == NULL) )
		return;

	// stop at these in case they ever get used, we need to support them at that point
	Assert( (dir == NULL) && (ext == NULL) );

	/* fs2 only uses fname */
	if (fname != NULL) {
		const char *ls = strrchr(path, '/');

		if (ls != NULL) {
			ls++;		// move past '/'
		} else {
			ls = path;
		}

		const char *lp = strrchr(path, '.');

		if (lp == NULL) {
			lp = ls + strlen(ls);	// move to the end
		}

		int dist = lp-ls;

		if (dist > (_MAX_FNAME-1))
			dist = _MAX_FNAME-1;

		strncpy(fname, ls, dist);
		fname[dist] = 0;	// add null, just in case
	}
}

int MulDiv(int number, int numerator, int denominator)
{
	int result;

	if (denominator == 0)
		return 0;

	longlong tmp;
	tmp = ((longlong) number) * ((longlong) numerator);
	tmp /= (longlong) denominator;
	result = (int) tmp;

	return result;
}

#endif // _WIN32
