/* This file contains debugging utilities used only under VC2005+
 * Maintained by: portej05 (please run patches past him before committing here!)
 */

/* You will need to define PDB_DEBUGGING in order to use these functions
 * Important note:
 *  When you call SCP_DumpStack, your thread will be suspended until the stack is dumped
 *  If you've got more than a single thread 
 */

/* Things you should be using from this header
 * SCP_IDumpHandler - implement this in order to receive symbols from DumpStack
 */

#ifndef _MSPDB_CALLSTACK_H_INCLUDED_
#define _MSPDB_CALLSTACK_H_INCLUDED_

#if defined(PDB_DEBUGGING)

/* Must have windows.h */
#include <windows.h>

#ifndef PURE
#	define PURE = 0
#endif

/* PUBLIC - use these */

/* This interface will be used by the stack tracing code
 *  to return symbols
 */
class SCP_IDumpHandler abstract
{
public:
	virtual bool ResolveSymbols( ) PURE;
	virtual void OnBegin( ) PURE;
	virtual void OnEntry( void* address, const char* module, const char* symbol ) PURE;
	virtual void OnError( const char* error ) PURE;
	virtual void OnEnd( ) PURE;
};

extern HRESULT SCP_DumpStack( SCP_IDumpHandler* pISH );

/* INTERNAL - please don't use these */
#define SCP_MSPDBCS_MAX_SYMBOL_LENGTH 1000
#define SCP_MSPDBCS_MAX_MODULE_LENGTH _MAX_PATH
#define SCP_MSPDBCS_MAX_STACK_FRAMES 100 /* arbitrary */

struct SCP_mspdbcs_SDumpStackThreadInfo
{
	HANDLE hThread;
	HANDLE hProcess;
	SCP_IDumpHandler* pIDS;
};

struct SCP_mspdbcs_SDumpStackSymbolInfo
{
	ULONG_PTR dwAddress;
	DWORD64 dwOffset; /* Will be truncated to DWORD under Win32 */
	char szModule[ SCP_MSPDBCS_MAX_MODULE_LENGTH ];
	char szSymbol[ SCP_MSPDBCS_MAX_SYMBOL_LENGTH ];
};

/* INTERNAL FUNCTIONS */
extern BOOL SCP_mspdbcs_ResolveSymbol( HANDLE hProcess, UINT_PTR dwAddress, SCP_mspdbcs_SDumpStackThreadInfo& siSymbol );
extern LPVOID __stdcall SCP_mspdbcs_FunctionTableAccess( HANDLE hProcess, DWORD64 dwPCAddress );
extern DWORD64 __stdcall SCP_mspdbcs_GetModuleBase( HANDLE hProcess, DWORD64 returnAddress );
extern DWORD WINAPI SCP_mspdbcs_DumpStackThread( LPVOID pv );

#endif // PDB_DEBUGGING

/* Initialisation */
extern void SCP_mspdbcs_Initialise( );
extern void SCP_mspdbcs_Cleanup( );

#endif // _MSPDB_CALLSTACK_H_INCLUDED_