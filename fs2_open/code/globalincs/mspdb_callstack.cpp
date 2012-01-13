/* This file contains debugging utilities used only under VC2005+
 * Maintained by: portej05 (please run patches past him before committing here!)
 */

/* Based on the ATL headers, however, updated to fix one or two bugs in the ATL headers
 *  and moved to the Sym*64 functions
 */

#if defined(PDB_DEBUGGING)

/* Windows */
#include <windows.h>
#include <dbghelp.h>

/* SCP */
#include "globalincs/pstypes.h"
#include "globalincs/mspdb_callstack.h"

/* Link the library that we need */
#pragma comment( lib, "dbghelp.lib" )

HRESULT SCP_DumpStack( SCP_IDumpHandler* pISH );
BOOL SCP_mspdbcs_ResolveSymbol( HANDLE hProcess, UINT_PTR dwAddress, SCP_mspdbcs_SDumpStackSymbolInfo& siSymbol );
LPVOID __stdcall SCP_mspdbcs_FunctionTableAccess( HANDLE hProcess, DWORD64 dwPCAddress );
DWORD64 __stdcall SCP_mspdbcs_GetModuleBase( HANDLE hProcess, DWORD64 returnAddress );
DWORD WINAPI SCP_mspdbcs_DumpStackThread( LPVOID pv );
void SCP_mspdbcs_Initialise( );
void SCP_mspdbcs_Cleanup( );

static bool SCP_mspdbcs_initialised = false;
static CRITICAL_SECTION SCP_mspdbcs_cs;

BOOL SCP_mspdbcs_ResolveSymbol( HANDLE hProcess, UINT_PTR dwAddress, SCP_mspdbcs_SDumpStackSymbolInfo& siSymbol )
{
	BOOL retVal = TRUE;
	
	char szUndec[ SCP_MSPDBCS_MAX_SYMBOL_LENGTH ];
	char szWithOffset[ SCP_MSPDBCS_MAX_SYMBOL_LENGTH ];
	char* pszSymbol = NULL;
	IMAGEHLP_MODULE64 mi;

	memset( &siSymbol, 0, sizeof( SCP_mspdbcs_SDumpStackSymbolInfo ) );
	mi.SizeOfStruct = sizeof( IMAGEHLP_MODULE64 );

	siSymbol.dwAddress = dwAddress;

	if ( !SymGetModuleInfo64( hProcess, dwAddress, &mi ) )
	{
		/* Unneeded */
		/*printf("Error: %x\n", HRESULT_FROM_WIN32( GetLastError( ) ));*/
		strcpy_s( siSymbol.szModule, "<no module>" );
	}
	else
	{
		char* pszLastModule = strchr( mi.ImageName, '\\' );

		if ( pszLastModule == NULL )
			pszLastModule = mi.ImageName;
		else
			pszLastModule++; /* move off the backslash */

		strncpy_s( siSymbol.szModule, pszLastModule, _TRUNCATE );
	}

	__try
	{
		union
		{
			CHAR rgchSymbol[ sizeof(IMAGEHLP_SYMBOL64) + SCP_MSPDBCS_MAX_SYMBOL_LENGTH ];
			IMAGEHLP_SYMBOL64 sym;
		} sym;

		memset( &sym.sym, 0, sizeof( sym.sym ) );
		sym.sym.SizeOfStruct = sizeof( IMAGEHLP_SYMBOL64 );

#ifdef _WIN64
		sym.sym.Address = dwAddress;
#else
		sym.sym.Address = (DWORD)dwAddress;
#endif
		sym.sym.MaxNameLength = SCP_MSPDBCS_MAX_SYMBOL_LENGTH;
		
#ifdef _WIN64
		if ( SymGetSymFromAddr64( hProcess, dwAddress, &(siSymbol.dwOffset), &sym.sym ) )
#else
		if ( SymGetSymFromAddr64( hProcess, (DWORD)dwAddress, &(siSymbol.dwOffset), &sym.sym ) )
#endif
		{
			pszSymbol = sym.sym.Name;

			if ( UnDecorateSymbolName( sym.sym.Name, szUndec, sizeof( szUndec )/sizeof( szUndec[0] ),
									   UNDNAME_NO_MS_KEYWORDS | UNDNAME_NO_ACCESS_SPECIFIERS ) )
			{
				pszSymbol = szUndec;
			}
			else if ( SymUnDName64( &sym.sym, szUndec, sizeof( szUndec )/sizeof( szUndec[0] ) ) )
			{
				pszSymbol = szUndec;
			}

			if ( siSymbol.dwOffset != 0 )
			{
				sprintf_s( szWithOffset, SCP_MSPDBCS_MAX_SYMBOL_LENGTH, "%s + %d bytes", pszSymbol, siSymbol.dwOffset );
				szWithOffset[ SCP_MSPDBCS_MAX_SYMBOL_LENGTH - 1 ] = '\0'; /* Because sprintf doesn't guarantee NULL terminating */
				pszSymbol = szWithOffset;
			}
		}
		else
			pszSymbol = "<no symbol>";
	}
	__except( EXCEPTION_ACCESS_VIOLATION == GetExceptionCode( ) )
	{
		pszSymbol = "<EX: no symbol>";
		siSymbol.dwOffset = dwAddress - mi.BaseOfImage;
	}

	strncpy_s( siSymbol.szSymbol, pszSymbol, _TRUNCATE );

	return retVal;
}

LPVOID __stdcall SCP_mspdbcs_FunctionTableAccess( HANDLE hProcess, DWORD64 dwPCAddress )
{
	return SymFunctionTableAccess64( hProcess, dwPCAddress );
}

DWORD64 __stdcall SCP_mspdbcs_GetModuleBase( HANDLE hProcess, DWORD64 returnAddress )
{
	IMAGEHLP_MODULE moduleInfo;
	moduleInfo.SizeOfStruct = sizeof( IMAGEHLP_MODULE );
	
	/* The ATL headers do it this way */
#ifdef _WIN64
	if ( SymGetModuleInfo( hProcess, returnAddress, &moduleInfo ) )
#else
	if ( SymGetModuleInfo( hProcess, (ULONG)returnAddress, &moduleInfo ) )
#endif
	{
		return moduleInfo.BaseOfImage;
	}
	else
	{
		MEMORY_BASIC_INFORMATION memoryBasicInfo;

		if ( VirtualQueryEx( hProcess, (LPVOID)returnAddress, 
							 &memoryBasicInfo, sizeof( MEMORY_BASIC_INFORMATION ) ) )
		{
			DWORD cch = 0;
			char szFile[ _MAX_PATH ] = {0}; /* Initialise the file path */
			cch = GetModuleFileNameA( (HINSTANCE)memoryBasicInfo.AllocationBase, szFile, MAX_PATH );
			SymLoadModule( hProcess, NULL, ((cch)?szFile:NULL),
#ifdef _WIN64
						   NULL, (DWORD_PTR)memoryBasicInfo.AllocationBase, 0 );
#else
						   NULL, (DWORD)(DWORD_PTR)memoryBasicInfo.AllocationBase, 0 );
#endif
			return (DWORD_PTR)memoryBasicInfo.AllocationBase;
		}
	}

	return NULL;
}

DWORD WINAPI SCP_mspdbcs_DumpStackThread( LPVOID pv )
{
	CONTEXT context;
	memset( &context, 0, sizeof( CONTEXT ) );
	context.ContextFlags = CONTEXT_FULL;
	SCP_mspdbcs_SDumpStackThreadInfo* pdsti = 
		reinterpret_cast< SCP_mspdbcs_SDumpStackThreadInfo* >( pv );

	/* We're going to walk the stack here */
	
	/* Suspend the running thread */
	SuspendThread( pdsti->hThread );
	
	pdsti->pIDS->OnBegin( );

	/* Retrieve the context (processor state) of the suspended thread */
	GetThreadContext( pdsti->hThread, &context );

	/* Set name decoration handling */
	DWORD dw = SymGetOptions( );
	dw = dw & ~SYMOPT_UNDNAME;
	SymSetOptions( dw );

	/* Initialise the stackframe */
	STACKFRAME64 stackFrame;
	memset( &stackFrame, 0, sizeof( STACKFRAME64 ) );
	stackFrame.AddrPC.Mode = AddrModeFlat;
	stackFrame.AddrFrame.Mode = AddrModeFlat;
	stackFrame.AddrStack.Mode = AddrModeFlat;
	stackFrame.AddrReturn.Mode = AddrModeFlat;
	stackFrame.AddrBStore.Mode = AddrModeFlat;

	DWORD dwMachType;

	/* Determine the machine type based on the compilation.
	 * Initialise stackFrame accordingly
	 * We will only handle the types of _M_IX86 or _M_AMD64
	 * We're not intending to be compatible with IA64 or any other architectures
	 *  under windows
	 */
#if defined(_M_IX86)
	dwMachType = IMAGE_FILE_MACHINE_I386;
	stackFrame.AddrPC.Offset = context.Eip;
	stackFrame.AddrStack.Offset = context.Esp;
	stackFrame.AddrFrame.Offset = context.Ebp;
#elif defined(_M_AMD64)
	dwMachType = IMAGE_FILE_MACHINE_AMD64;
	stackFrame.AddrPC.Offset = context.Rip;
	stackFrame.AddrStack = context.Rsp;
#else
#		error UNKNOWN ARCHITECTURE
#endif

	/* All the discovered addresses will be stored in an array */
	SCP_vector< void* > addresses;

	/* Walk the stack */
	for ( int currFrame = 0; currFrame < SCP_MSPDBCS_MAX_STACK_FRAMES; currFrame++ )
	{
		if ( !StackWalk64( dwMachType, pdsti->hProcess, pdsti->hThread,
						   &stackFrame, &context, NULL,
						   SCP_mspdbcs_FunctionTableAccess, SCP_mspdbcs_GetModuleBase, NULL ) )
		{
			break; /* No more stack frames to walk */
		}

		/* Found a useful address? */
		if ( stackFrame.AddrPC.Offset != 0 )
		{
			if ( pdsti->pIDS->ResolveSymbols( ) )
				addresses.push_back( (void*)(DWORD_PTR)stackFrame.AddrPC.Offset );
			else
				pdsti->pIDS->OnEntry( (void*)(DWORD_PTR)stackFrame.AddrPC.Offset, NULL, NULL );
		}
	}

	if ( pdsti->pIDS->ResolveSymbols( ) )
	{
		/* Dump the stack information that we have */
		for ( size_t i = 0; i < addresses.size( ); i++ )
		{
			SCP_mspdbcs_SDumpStackSymbolInfo info;
			const char* szModule = NULL;
			const char* szSymbol = NULL;

			if ( SCP_mspdbcs_ResolveSymbol( pdsti->hProcess, (UINT_PTR)addresses[ i ], info ) )
			{
				szModule = info.szModule;
				szSymbol = info.szSymbol;
			}

			pdsti->pIDS->OnEntry( addresses[ i ], szModule, szSymbol );
		}
	}

	pdsti->pIDS->OnEnd( );
	ResumeThread( pdsti->hThread );

	return 0;
}

/* Entry point */
HRESULT SCP_DumpStack( SCP_IDumpHandler* pIDH )
{
	if ( !pIDH )
		return E_POINTER;

	/* Retrieve pseudo handles to the current thread and process */
	HANDLE hPseudoThread = GetCurrentThread( );	
	HANDLE hPseudoProcess = GetCurrentProcess( );

	/* Retrieve the real handle of this thread */
	HANDLE hThread = NULL;
	if ( !DuplicateHandle( hPseudoProcess, hPseudoThread, /* Source process and thread */
						   hPseudoProcess, &hThread, /* Target process and thread */
						   0, FALSE, DUPLICATE_SAME_ACCESS ) ) /* Non-Inheritable, same access as current process/thread */
		return HRESULT_FROM_WIN32( GetLastError( ) ); /* Bugger */

	DWORD dwID;
	
	SCP_mspdbcs_SDumpStackThreadInfo info;
	info.hProcess = hPseudoProcess;
	info.hThread = hThread;
	info.pIDS = pIDH;
	
	/* Calls to dbghelp.dll must be synchronised :( */
	EnterCriticalSection( &SCP_mspdbcs_cs );

	/* This will fail if SymInitialize hasn't been called, so 
	 *  this protects against uninitialised state */
	if ( !SCP_mspdbcs_initialised )
	{
		LeaveCriticalSection( &SCP_mspdbcs_cs );
		mprintf( ("Symbols not initialised\n") );
		return E_UNEXPECTED;
	}

	HANDLE workerThread = CreateThread( NULL, 0, SCP_mspdbcs_DumpStackThread,
										&info, 0, &dwID );

	LeaveCriticalSection( &SCP_mspdbcs_cs );

	if ( workerThread == NULL )
		return HRESULT_FROM_WIN32( GetLastError( ) ); /* Bugger */

	while ( WaitForSingleObject( workerThread, 0 ) != WAIT_OBJECT_0 );

	CloseHandle( workerThread );
	return S_OK;
}

#endif // PDB_DEBUGGING

void SCP_mspdbcs_Initialise( )
{
#ifdef PDB_DEBUGGING
	HANDLE hPseudoProcess = GetCurrentProcess( );
	if ( !SymInitialize( hPseudoProcess, NULL, TRUE ) )
	{
		mprintf( ("Could not initialise symbols - callstacks will fail: %x\n", HRESULT_FROM_WIN32( GetLastError( ) ) ) );
	}
	else
	{
		InitializeCriticalSection( &SCP_mspdbcs_cs );
		SCP_mspdbcs_initialised = true;
	}
#endif
}

void SCP_mspdbcs_Cleanup( )
{
#ifdef PDB_DEBUGGING
	/* We enter the critical section to synchronise the check of
	 *  SCP_mspdbcs_initialised. Failure to do that could cause
	 *  SymCleanup to be called before SCP_dump_stack is finished
	 */
	EnterCriticalSection( &SCP_mspdbcs_cs );
	SCP_mspdbcs_initialised = false; /* stop problems at end of execution */
	LeaveCriticalSection( &SCP_mspdbcs_cs );

	DeleteCriticalSection( &SCP_mspdbcs_cs );

	HANDLE hPseudoProcess = GetCurrentProcess( );
	SymCleanup( hPseudoProcess );
#endif
}