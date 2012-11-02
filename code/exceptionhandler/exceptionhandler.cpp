/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



// Copyright © 1998 Bruce Dawson.
/*
This source file contains the exception handler for recording error
information after crashes. See exceptionhandler.h for information
on how to hook it in.
*/

#ifdef GAME_ERRORLOG_TXT

#ifdef _WIN32

#include "globalincs/pstypes.h"

#include <windows.h>


// --------------------
//
// Defines
//
// --------------------
#define	ONEK			1024
#define	SIXTYFOURK		(64*ONEK)
#define	ONEM			(ONEK*ONEK)
#define	ONEG			(ONEK*ONEK*ONEK) 

// --------------------
//
// Enumerated Types
//
// --------------------


// --------------------
//
// Structures
//
// --------------------


// --------------------
//
// Classes
//
// --------------------


// --------------------
//
// Global Variables
//
// --------------------


// --------------------
//
// Local Variables
//
// --------------------

const int NumCodeBytes = 16;	// Number of code bytes to record.
const int MaxStackDump = 2048;	// Maximum number of DWORDS in stack dumps.
const int StackColumns = 8;		// Number of columns in stack dump.

// --------------------
//
// Internal Functions
//
// --------------------

// hprintf behaves similarly to printf, with a few vital differences.
// It uses wvsprintf to do the formatting, which is a system routine,
// thus avoiding C run time interactions. For similar reasons it
// uses WriteFile rather than fwrite.
// The one limitation that this imposes is that wvsprintf, and
// therefore hprintf, cannot handle floating point numbers.
static void hprintf(HANDLE LogFile, char* Format, ...)
{
	char buffer[2000];	// wvsprintf never prints more than one K.

	va_list arglist;
	va_start( arglist, Format);
	wvsprintf(buffer, Format, arglist);
	va_end( arglist);

	DWORD NumBytes;
	WriteFile(LogFile, buffer, lstrlen(buffer), &NumBytes, 0);
}

// Print the specified FILETIME to output in a human readable format,
// without using the C run time.
static void PrintTime(char *output, FILETIME TimeToPrint)
{
	WORD Date, Time;
	if (FileTimeToLocalFileTime(&TimeToPrint, &TimeToPrint) &&
				FileTimeToDosDateTime(&TimeToPrint, &Date, &Time))
	{
		// What a silly way to print out the file date/time. Oh well,
		// it works, and I'm not aware of a cleaner way to do it.
		wsprintf(output, "%d/%d/%d %02d:%02d:%02d",
					(Date / 32) & 15, Date & 31, (Date / 512) + 1980,
					(Time / 2048), (Time / 32) & 63, (Time & 31) * 2);
	} else {
		output[0] = 0;
	}
}

// Print information about a code module (DLL or EXE) such as its size,
// location, time stamp, etc.
static void ShowModuleInfo(HANDLE LogFile, HINSTANCE ModuleHandle)
{
	char ModName[MAX_PATH];
#ifdef _MSC_VER
	__try {
#endif
		if (GetModuleFileName(ModuleHandle, ModName, sizeof(ModName)) > 0) {
			// If GetModuleFileName returns greater than zero then this must
			// be a valid code module address. Therefore we can try to walk
			// our way through its structures to find the link time stamp.
			IMAGE_DOS_HEADER *DosHeader = (IMAGE_DOS_HEADER*)ModuleHandle;
			if (IMAGE_DOS_SIGNATURE != DosHeader->e_magic) {
	    	    return;
			}

			IMAGE_NT_HEADERS *NTHeader = (IMAGE_NT_HEADERS*)((char *)DosHeader + DosHeader->e_lfanew);
			if (IMAGE_NT_SIGNATURE != NTHeader->Signature) {
	    	    return;
			}

			// Open the code module file so that we can get its file date
			// and size.
			HANDLE ModuleFile = CreateFile(ModName, GENERIC_READ, 
						FILE_SHARE_READ, 0, OPEN_EXISTING,
						FILE_ATTRIBUTE_NORMAL, 0);
			char TimeBuffer[100] = "";
			DWORD FileSize = 0;
			if (ModuleFile != INVALID_HANDLE_VALUE) {
				FileSize = GetFileSize(ModuleFile, 0);
				FILETIME	LastWriteTime;
				if (GetFileTime(ModuleFile, 0, 0, &LastWriteTime)) {
					wsprintf(TimeBuffer, " - file date is ");
					PrintTime(TimeBuffer + lstrlen(TimeBuffer), LastWriteTime);
				}
				CloseHandle(ModuleFile);
			}
			hprintf(LogFile, "%s, loaded at 0x%08x - %d bytes - %08x%s\r\n",
						ModName, ModuleHandle, FileSize,
						NTHeader->FileHeader.TimeDateStamp, TimeBuffer);
		}
#ifdef _MSC_VER
	}
	// Handle any exceptions by continuing from this point.
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
	}
#endif
}

// Scan memory looking for code modules (DLLs or EXEs). VirtualQuery is used
// to find all the blocks of address space that were reserved or committed,
// and ShowModuleInfo will display module information if they are code
// modules.

static void RecordModuleList(HANDLE LogFile)
{
	hprintf(LogFile, "\r\n"
					 "\tModule list: names, addresses, sizes, time stamps "
			"and file times:\r\n");
	SYSTEM_INFO	SystemInfo;
	GetSystemInfo(&SystemInfo);
	const size_t PageSize = SystemInfo.dwPageSize;
	// Set NumPages to the number of pages in the 4GByte address space,
	// while being careful to avoid overflowing ints.
	const size_t NumPages = 4 * size_t(ONEG / PageSize);
	size_t pageNum = 0;
	void *LastAllocationBase = 0;
	while (pageNum < NumPages) {
		MEMORY_BASIC_INFORMATION	MemInfo;
		if (VirtualQuery((void *)(pageNum * PageSize), &MemInfo, sizeof(MemInfo))) {
			if (MemInfo.RegionSize > 0) {

				// Adjust the page number to skip over this block of memory.
				pageNum += MemInfo.RegionSize / PageSize;
				if (MemInfo.State == MEM_COMMIT && MemInfo.AllocationBase > LastAllocationBase) {
					// Look for new blocks of committed memory, and try
					// recording their module names - this will fail
					// gracefully if they aren't code modules.
					LastAllocationBase = MemInfo.AllocationBase;
					ShowModuleInfo(LogFile, (HINSTANCE)LastAllocationBase);
				}
			} else {
				pageNum += SIXTYFOURK / PageSize;
			}
		} else {
			// If VirtualQuery fails we advance by 64K because that is the
			// granularity of address space doled out by VirtualAlloc().
			pageNum += SIXTYFOURK / PageSize;
		}
	}
}

// Record information about the user's system, such as processor type, amount
// of memory, etc.

static void RecordSystemInformation(HANDLE LogFile)
{
	FILETIME	CurrentTime;
	GetSystemTimeAsFileTime(&CurrentTime);
	char TimeBuffer[100];
	PrintTime(TimeBuffer, CurrentTime);
	hprintf(LogFile, "Error occurred at %s.\r\n", TimeBuffer);
	char	ModuleName[MAX_PATH];
	if (GetModuleFileName(0, ModuleName, sizeof(ModuleName)) <= 0) {
		lstrcpy(ModuleName, "Unknown");
	}
	char	UserName[200];
	DWORD UserNameSize = sizeof(UserName);
	if (!GetUserName(UserName, &UserNameSize)) {
		lstrcpy(UserName, "Unknown");
	}
	hprintf(LogFile, "%s, run by %s.\r\n", ModuleName, UserName);

	SYSTEM_INFO	SystemInfo;
	GetSystemInfo(&SystemInfo);
	hprintf(LogFile, "%d processor(s), type %d.\r\n",
				SystemInfo.dwNumberOfProcessors, SystemInfo.dwProcessorType);

	MEMORYSTATUS	MemInfo;
	MemInfo.dwLength = sizeof(MemInfo);
	GlobalMemoryStatus(&MemInfo);
	// Print out the amount of physical memory, rounded up.
	hprintf(LogFile, "%d MBytes physical memory.\r\n", (MemInfo.dwTotalPhys +
				ONEM - 1) / ONEM);
}

// Translate the exception code into something human readable.

static const char *GetExceptionDescription(DWORD ExceptionCode)
{
	struct ExceptionNames
	{
		DWORD	ExceptionCode;
		char*	ExceptionName;
	};

	ExceptionNames ExceptionMap[] =
	{
		{0x40010005, "a Control-C"},
		{0x40010008, "a Control-Break"},
		{0x80000002, "a Datatype Misalignment"},
		{0x80000003, "a Breakpoint"},
		{0xc0000005, "an Access Violation"},
		{0xc0000006, "an In Page Error"},
		{0xc0000017, "a No Memory"},
		{0xc000001d, "an Illegal Instruction"},
		{0xc0000025, "a Noncontinuable Exception"},
		{0xc0000026, "an Invalid Disposition"},
		{0xc000008c, "a Array Bounds Exceeded"},
		{0xc000008d, "a Float Denormal Operand"},
		{0xc000008e, "a Float Divide by Zero"},
		{0xc000008f, "a Float Inexact Result"},
		{0xc0000090, "a Float Invalid Operation"},
		{0xc0000091, "a Float Overflow"},
		{0xc0000092, "a Float Stack Check"},
		{0xc0000093, "a Float Underflow"},
		{0xc0000094, "an Integer Divide by Zero"},
		{0xc0000095, "an Integer Overflow"},
		{0xc0000096, "a Privileged Instruction"},
		{0xc00000fD, "a Stack Overflow"},
		{0xc0000142, "a DLL Initialization Failed"},
		{0xe06d7363, "a Microsoft C++ Exception"},
	};

	for (int i = 0; i < sizeof(ExceptionMap) / sizeof(ExceptionMap[0]); i++) {
		if (ExceptionCode == ExceptionMap[i].ExceptionCode) {
			return ExceptionMap[i].ExceptionName;
		}
	}

	return "Unknown exception type";
}

static char* GetFilePart(char *source)
{
	char *result = strrchr(source, '\\');
	if (result) {
		result++;
	} else {
		result = source;
	}
	return result;
}

// --------------------
//
// External Functions
//
// --------------------

SCP_string safe_string;

// Entry point into the main exception handling routine.  This routine is put into an except()
// statment at the beginning of a thread and is called anytime that there is a program exception
// The data is stored in a file called ErrorLog.txt in the data directory.
//
// data:			pointer to the exception data
// Message:		Any message	that should be printed out in the error log file
//
// returns: 
//
int __cdecl RecordExceptionInfo(PEXCEPTION_POINTERS data, const char *Message)
{
	static bool BeenHere = false;

	// Going recursive! That must mean this routine crashed!
	if (BeenHere) {
		return EXCEPTION_CONTINUE_SEARCH;
	}

	BeenHere = true;

	char	ModuleName[MAX_PATH];
	char	FileName[MAX_PATH] = "Unknown";
	// Create a filename to record the error information to.
	// Storing it in the executable directory works well.
	if (GetModuleFileName(0, ModuleName, sizeof(ModuleName)) <= 0) {
		ModuleName[0] = 0;
	}

	char *FilePart = GetFilePart(ModuleName);

	// Extract the file name portion and remove it's file extension. We'll
	// use that name shortly.
	lstrcpy(FileName, FilePart);
	char *lastperiod = strrchr(FileName, '.');
	if (lastperiod) {
		lastperiod[0] = 0;
	}

	// Replace the executable filename with our error log file name.
	lstrcpy(FilePart, "errorlog.txt");
	HANDLE LogFile = CreateFile(ModuleName, GENERIC_WRITE, 0, 0,
				OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH, 0);
	if (LogFile == INVALID_HANDLE_VALUE) {
		OutputDebugString("Error creating exception report");
		return EXCEPTION_CONTINUE_SEARCH;
	}

	// Append to the error log.
	SetFilePointer(LogFile, 0, 0, FILE_END);
	// Print out some blank lines to separate this error log from any previous ones.
	hprintf(LogFile, "\r\n\r\n\r\n\r\n");
	PEXCEPTION_RECORD	Exception = data->ExceptionRecord;
	PCONTEXT			Context = data->ContextRecord;

	char	CrashModulePathName[MAX_PATH];
	char	*CrashModuleFileName = "Unknown";
	MEMORY_BASIC_INFORMATION	MemInfo;
	// VirtualQuery can be used to get the allocation base associated with a
	// code address, which is the same as the ModuleHandle. This can be used
	// to get the filename of the module that the crash happened in.
	if (VirtualQuery((void*)Context->Eip, &MemInfo, sizeof(MemInfo)) && GetModuleFileName((HINSTANCE)MemInfo.AllocationBase, CrashModulePathName, sizeof(CrashModulePathName)) > 0) {
		CrashModuleFileName = GetFilePart(CrashModulePathName);
	}

	// Print out the beginning of the error log in a Win95 error window
	// compatible format.
	hprintf(LogFile, "%s caused %s in module %s at %04x:%08x.\r\n",
				FileName, GetExceptionDescription(Exception->ExceptionCode),
				CrashModuleFileName, Context->SegCs, Context->Eip);
	hprintf(LogFile, "Exception handler called in %s.\r\n", Message);
	RecordSystemInformation(LogFile);
	// If the exception was an access violation, print out some additional
	// information, to the error log and the debugger.
	if (Exception->ExceptionCode == STATUS_ACCESS_VIOLATION && Exception->NumberParameters >= 2) {
		char DebugMessage[1000];
		const char* readwrite = "Read from";
		if (Exception->ExceptionInformation[0]) {
			readwrite = "Write to";
		}

		wsprintf(DebugMessage, "%s location %08x caused an access violation.\r\n", readwrite, Exception->ExceptionInformation[1]);

#ifdef	_DEBUG
		// The VisualC++ debugger doesn't actually tell you whether a read
		// or a write caused the access violation, nor does it tell what
		// address was being read or written. So I fixed that.
		OutputDebugString("Exception handler: ");
		OutputDebugString(DebugMessage);
#endif

		hprintf(LogFile, "%s", DebugMessage);
	}

	// Print out the register values in a Win95 error window compatible format.
	hprintf(LogFile, "\r\n");
	hprintf(LogFile, "Registers:\r\n");
	hprintf(LogFile, "EAX=%08x CS=%04x EIP=%08x EFLGS=%08x\r\n",
				Context->Eax, Context->SegCs, Context->Eip, Context->EFlags);
	hprintf(LogFile, "EBX=%08x SS=%04x ESP=%08x EBP=%08x\r\n",
				Context->Ebx, Context->SegSs, Context->Esp, Context->Ebp);
	hprintf(LogFile, "ECX=%08x DS=%04x ESI=%08x FS=%04x\r\n",
				Context->Ecx, Context->SegDs, Context->Esi, Context->SegFs);
	hprintf(LogFile, "EDX=%08x ES=%04x EDI=%08x GS=%04x\r\n",
				Context->Edx, Context->SegEs, Context->Edi, Context->SegGs);
	hprintf(LogFile, "Bytes at CS:EIP:\r\n");

	// Print out the bytes of code at the instruction pointer. Since the
	// crash may have been caused by an instruction pointer that was bad,
	// this code needs to be wrapped in an exception handler, in case there
	// is no memory to read. If the dereferencing of code[] fails, the
	// exception handler will print '??'.
	unsigned char *code = (unsigned char*)Context->Eip;
	for (int codebyte = 0; codebyte < NumCodeBytes; codebyte++) {
#ifdef _MSC_VER
		__try {
#endif
			hprintf(LogFile, "%02x ", code[codebyte]);
#ifdef _MSC_VER
		}
		__except(EXCEPTION_EXECUTE_HANDLER) {
			hprintf(LogFile, "?? ");
		}
#endif
	}

#ifdef _MSC_VER
	// Time to print part or all of the stack to the error log. This allows
	// us to figure out the call stack, parameters, local variables, etc.
	hprintf(LogFile, "\r\n"
					 "Stack dump:\r\n");
	__try {
		// Esp contains the bottom of the stack, or at least the bottom of
		// the currently used area.
		DWORD* pStack = (DWORD *)Context->Esp;
		DWORD* pStackTop;
		__asm
		{
			// Load the top (highest address) of the stack from the
			// thread information block. It will be found there in
			// Win9x and Windows NT.
			mov	eax, fs:[4]
			mov pStackTop, eax
		}
		if (pStackTop > pStack + MaxStackDump) {
			pStackTop = pStack + MaxStackDump;
		}

		int Count = 0;
		// Too many calls to WriteFile can take a long time, causing
		// confusing delays when programs crash. Therefore I implemented
		// simple buffering for the stack dumping code instead of calling
		// hprintf directly.
		char	buffer[1000] = "";
		const int safetyzone = 50;
		char*	nearend = buffer + sizeof(buffer) - safetyzone;
		char*	output = buffer;
		while (pStack + 1 <= pStackTop) 	{
			if ((Count % StackColumns) == 0) {
				output += wsprintf(output, "%08x: ", pStack);
			}

			char *Suffix = " ";
			if ((++Count % StackColumns) == 0 || pStack + 2 > pStackTop) {
				Suffix = "\r\n";
			}

			output += wsprintf(output, "%08x%s", *pStack, Suffix);
			pStack++;
			// Check for when the buffer is almost full, and flush it to disk.
			if (output > nearend) {
				hprintf(LogFile, "%s", buffer);
				buffer[0] = 0;
				output = buffer;
			}
		}
		// Print out any final characters from the cache.
		hprintf(LogFile, "%s", buffer);
	}
	__except(EXCEPTION_EXECUTE_HANDLER) {
		hprintf(LogFile, "Exception encountered during stack dump.\r\n");
	}
#endif

#ifndef NDEBUG
	if (!safe_string.empty())
		hprintf(LogFile, "Last safe point: %s\r\n", safe_string.c_str());
#endif

	RecordModuleList(LogFile);

	CloseHandle(LogFile);
	// Return the magic value which tells Win32 that this handler didn't
	// actually handle the exception - so that things will proceed as per
	// normal.

	return EXCEPTION_CONTINUE_SEARCH;
}

#endif // _WIN32
#endif
