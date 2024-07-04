/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell
 * or otherwise commercially exploit the source or things you created based on the
 * source.
 *
*/

// Nothing in this module should be externalized!!!
//XSTR:OFF

//#define DUMPRAM	// This dumps all symbol sizes. See John for more info

/* Windows Headers */
#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <iomanip>

#ifdef _MSC_VER
#	include <crtdbg.h>
	/* Uncomment SHOW_CALL_STACK to show the call stack in Asserts, Warnings, and Errors */
#	define SHOW_CALL_STACK
#endif // _MSC_VER

/* STL Headers */
#include <string>

/* SCP Headers */
#include "osapi/osapi.h"
#include "globalincs/pstypes.h"
#include "globalincs/systemvars.h"
#include "cmdline/cmdline.h"
#include "parse/parselo.h"
#include "debugconsole/console.h"

#if defined( SHOW_CALL_STACK ) && defined( PDB_DEBUGGING )
#	include "globalincs/mspdb_callstack.h"
#endif

extern void gr_activate(int active);

#ifndef _ASSERT
  #ifndef _DEBUG
    #define _ASSERT(expr) ((void)0)
  #else
    #define _ASSERT(expr) (assert(expr))
//    #error _ASSERT is not defined yet for debug mode with non-MSVC compilers
  #endif
#endif

const char *clean_filename( const char *name)
{
	const char *p = name+strlen(name)-1;
	// Move p to point to first letter of EXE filename
	while( (*p!='\\') && (*p!='/') && (*p!=':') && (p>= name) )
		p--;
	p++;

	return p;
}


/* MSVC2005+ callstack support
 */
#if defined( SHOW_CALL_STACK ) && defined( PDB_DEBUGGING )

class SCP_DebugCallStack : public SCP_IDumpHandler
{
public:
	virtual bool ResolveSymbols( )
	{
		return true;
	}

	virtual void OnBegin( )
	{
	}

	virtual void OnEnd( )
	{
	}

	virtual void OnEntry( void* address, const char* module, const char* symbol )
	{
		UNREFERENCED_PARAMETER( address );

		StackEntry entry;
		entry.module = clean_filename( module );
		entry.symbol = symbol;
		m_stackFrames.push_back( entry );
	}

	virtual void OnError( const char* error )
	{
		/* No error handling here! */
		UNREFERENCED_PARAMETER( error );
	}

	SCP_string DumpToString( )
	{
		SCP_string callstack;
		for ( size_t i = 0; i < m_stackFrames.size( ); i++ )
		{
			callstack += m_stackFrames[ i ].module + "! " + m_stackFrames[ i ].symbol + "\n";
		}

		return callstack; /* Inefficient, but we don't need efficient here */
	}
private:
	struct StackEntry
	{
		SCP_string module;
		SCP_string symbol;
	};

	SCP_vector< StackEntry > m_stackFrames;
};

#elif defined( SHOW_CALL_STACK )

class PE_Debug
  {
  public :
    PE_Debug() ;
    ~PE_Debug() ;
    void ClearReport() ;
    int DumpDebugInfo( std::ostream& dumpBuffer, const BYTE* caller, HINSTANCE hInstance ) ;
  private :
    // Report data
    enum { MAX_MODULENAME_LEN = 512, VA_MAX_FILENAME_LEN = 256 } ;
    char latestModule[ MAX_MODULENAME_LEN ] ;
    char latestFile[ VA_MAX_FILENAME_LEN ] ;
    // File mapping data
    HANDLE hFile ;
    HANDLE hFileMapping ;
    PIMAGE_DOS_HEADER fileBase ;
    // Pointers to debug information
    PIMAGE_NT_HEADERS NT_Header ;
    PIMAGE_COFF_SYMBOLS_HEADER COFFDebugInfo ;
    PIMAGE_SYMBOL COFFSymbolTable ;
    int COFFSymbolCount ;
    const char* stringTable ;

    void ClearFileCache() ;
    void ClearDebugPtrs() ;
    void MapFileInMemory( const char* module ) ;
    void FindDebugInfo() ;
    void DumpSymbolInfo( std::ostream& dumpBuffer, DWORD relativeAddress ) ;
    void DumpLineNumber(std::ostream& dumpBuffer, DWORD relativeAddress ) ;
    PIMAGE_COFF_SYMBOLS_HEADER GetDebugHeader() ;
    PIMAGE_SECTION_HEADER SectionHeaderFromName( const char* name ) ;
    const char* GetSymbolName( PIMAGE_SYMBOL sym ) ;
  } ;


// Add an offset to a pointer and cast to a given type; may be
// implemented as a template function but Visual C++ has some problems.
#define BasedPtr( type, ptr, ofs ) (type)( (DWORD)(ptr) + (DWORD)(ofs) )


PE_Debug :: PE_Debug()
  {
  // Init file mapping cache
  hFileMapping = 0 ;
  hFile = INVALID_HANDLE_VALUE ;
  fileBase = 0 ;
  ClearDebugPtrs() ;
  ClearReport() ;
  }


PE_Debug :: ~PE_Debug()
  {
  ClearFileCache() ;
  }


void PE_Debug :: ClearReport()
  {
  latestModule[ 0 ] = 0 ;
  latestFile[ 0 ] = 0 ;
  }


void PE_Debug :: ClearDebugPtrs()
  {
  NT_Header = NULL ;
  COFFDebugInfo = NULL ;
  COFFSymbolTable = NULL ;
  COFFSymbolCount = 0 ;
  stringTable = NULL ;
  }


void PE_Debug :: ClearFileCache()
  {
  if( fileBase )
    {
    UnmapViewOfFile( fileBase ) ;
    fileBase = 0 ;
    }
  if( hFileMapping != 0 )
    {
    CloseHandle( hFileMapping ) ;
    hFileMapping = 0 ;
    }
  if( hFile != INVALID_HANDLE_VALUE )
    {
    CloseHandle( hFile ) ;
    hFile = INVALID_HANDLE_VALUE ;
    }
  }


void PE_Debug :: DumpLineNumber(std::ostream& dumpBuffer, DWORD relativeAddress )
  {
  PIMAGE_LINENUMBER line = BasedPtr( PIMAGE_LINENUMBER, COFFDebugInfo,
                                     COFFDebugInfo->LvaToFirstLinenumber ) ;
  DWORD lineCount = COFFDebugInfo->NumberOfLinenumbers ;
  const DWORD none = (DWORD)-1 ;
  DWORD maxAddr = 0 ;
  DWORD lineNum = none ;
  for( DWORD i=0; i < lineCount; i++ )
    {
    if( line->Linenumber != 0 )  // A regular line number
      {
      // look for line with bigger address <= relativeAddress
      if( line->Type.VirtualAddress <= relativeAddress &&
          line->Type.VirtualAddress > maxAddr )
        {
        maxAddr = line->Type.VirtualAddress ;
        lineNum = line->Linenumber ;
        }
      }
    line++ ;
    }
  if( lineNum != none ) {
	  dumpBuffer << "  line " << lineNum << "\n";
  }
  }


const char* PE_Debug :: GetSymbolName( PIMAGE_SYMBOL sym )
  {
  const int NAME_MAX_LEN = 64 ;
  static char buf[ NAME_MAX_LEN ] ;
  if( sym->N.Name.Short != 0 )
    {
    strncpy( buf, (const char*)sym->N.ShortName, 8 ) ;
    buf[ 8 ] = 0 ;
    }
  else
    {
    strncpy( buf, stringTable + sym->N.Name.Long, NAME_MAX_LEN ) ;
    buf[ NAME_MAX_LEN - 1 ] = 0 ;
    }
  return( buf ) ;
  }

void unmangle(char *dst, const char *src)
{
	//strcpy_s( dst, src );
	//return;

	src++;
	while( (*src) && (*src!=' ') && (*src!='@') )	{
		*dst++ = *src++;
	}
	*dst++ = 0;
}

#ifdef DUMPRAM

typedef struct MemSymbol {
	int	section;
	int	offset;
	int	size;
	char	name[132];
} MemSymbol;

int Num_symbols = 0;
int Max_symbols = 0;
MemSymbol *Symbols;

void InitSymbols()
{
	Num_symbols = 0;
	Max_symbols = 5000;
	Symbols = (MemSymbol *)vm_malloc(Max_symbols*sizeof(MemSymbol));
	if ( !Symbols )	{
		Max_symbols = 0;
	}
}

void Add_Symbol( int section, int offset, const char *name, char *module )
{
	if ( Num_symbols >= Max_symbols ) {
		return;
	}

	MemSymbol * sym = &Symbols[Num_symbols++];

	sym->section = section;
	sym->offset = offset;
	sym->size = -1;

	strcpy_s( sym->name, name );
	strcat_s( sym->name, "(" );
	strcat_s( sym->name, module );
	strcat_s( sym->name, ")" );

}

int Sym_compare( const MemSymbol *sym1, const MemSymbol *sym2 )
{
	if ( sym1->section < sym2->section )	{
		return -1;
	} else if ( sym1->section > sym2->section ) {
		return 1;
	} else {
		if ( sym1->offset > sym2->offset )	{
			return 1;
		} else {
			return -1;
		}
	}
}

int Sym_compare1( const MemSymbol *sym1, const MemSymbol *sym2 )
{
	if ( sym1->size < sym2->size )	{
		return 1;
	} else if ( sym1->size > sym2->size ) {
		return -1;
	} else {
		return 0;
	}
}

void DumpSymbols()
{
	int i;

	insertion_sort( Symbols, Num_symbols, Sym_compare );

	for (i=0;i<Num_symbols; i++ )	{
		MemSymbol * sym1 = &Symbols[i];
		MemSymbol * sym2 = &Symbols[i+1];
		if ( (i<Num_symbols-1) && (sym1->section == sym2->section) )	{
			sym1->size = sym2->offset-sym1->offset;
		} else {
			sym1->size = -1;
		}
	}

	insertion_sort( Symbols, Num_symbols, Sym_compare1 );


	FILE *fp = fopen( "dump", "wt" );

	fprintf( fp, "%-100s %10s %10s\n", "Name", "Size", "Total" );

	int total_size = 0;
	for (i=0;i<Num_symbols; i++ )	{
		MemSymbol * sym = &Symbols[i];
		if ( sym->size > 0 )
			total_size += sym->size;
		fprintf( fp, "%-100s %10d %10d\n", sym->name, sym->size, total_size );
	}

	fclose(fp);

	vm_free( Symbols );
	Symbols = NULL;
	_asm int 3
}
#endif

void PE_Debug::DumpSymbolInfo(std::ostream& dumpBuffer, DWORD relativeAddress )
{
	// Variables to keep track of function symbols
	PIMAGE_SYMBOL currentSym = COFFSymbolTable ;
	PIMAGE_SYMBOL fnSymbol = NULL ;
	DWORD maxFnAddress = 0 ;

	#ifdef DUMPRAM
	InitSymbols();
	#endif

	// Variables to keep track of file symbols
	PIMAGE_SYMBOL fileSymbol = NULL ;
	PIMAGE_SYMBOL latestFileSymbol = NULL ;
	for ( int i = 0; i < COFFSymbolCount; i++ )	{

		// Look for .text section where relativeAddress belongs to.
		// Keep track of the filename the .text section belongs to.
		if ( currentSym->StorageClass == IMAGE_SYM_CLASS_FILE )	{
			latestFileSymbol = currentSym;
		}

		// Borland uses "CODE" instead of the standard ".text" entry
		// Microsoft uses sections that only _begin_ with .text
		const char* symName = GetSymbolName( currentSym ) ;

		if ( strnicmp( symName, ".text", 5 ) == 0 || strcmpi( symName, "CODE" ) == 0 )	{
			if ( currentSym->Value <= relativeAddress )	{
				PIMAGE_AUX_SYMBOL auxSym = (PIMAGE_AUX_SYMBOL)(currentSym + 1) ;
				if ( currentSym->Value + auxSym->Section.Length >= relativeAddress )	{
					fileSymbol = latestFileSymbol ;
				}
			}
		}


		// Look for the function with biggest address <= relativeAddress
		BOOL isFunction = ISFCN( currentSym->Type ); // Type == 0x20, See WINNT.H
		if ( isFunction && ( currentSym->StorageClass == IMAGE_SYM_CLASS_EXTERNAL || currentSym->StorageClass == IMAGE_SYM_CLASS_STATIC ) )	{

			if ( currentSym->Value <= relativeAddress && currentSym->Value > maxFnAddress )	{
				maxFnAddress = currentSym->Value ;
				fnSymbol = currentSym ;
			}
		}

	#ifdef DUMPRAM
		if ( !isFunction && (currentSym->SectionNumber >= 0) )	{
			if ( (symName[0]=='_' && symName[1]!='$') || (symName[0]=='?') ) {

				char pretty_module[1024];

				if ( fileSymbol )	{
					const char* auxSym = (const char*)(latestFileSymbol + 1) ;
					char tmpFile[ VA_MAX_FILENAME_LEN ] ;
					strcpy_s( tmpFile, auxSym ) ;
					strcpy_s( pretty_module, tmpFile );
					char *p = pretty_module+strlen(pretty_module)-1;
					// Move p to point to first letter of EXE filename
					while( (*p!='\\') && (*p!='/') && (*p!=':') )
						p--;
					p++;
					if ( strlen(p) < 1 ) {
						strcpy_s( pretty_module, "<unknown>" );
					} else {
						memmove( pretty_module, p, strlen(p)+1 );
					}
				} else {
					strcpy_s( pretty_module, "" );
				}

				Add_Symbol( currentSym->SectionNumber, currentSym->Value, symName, pretty_module );
			}
		}
	#endif

		// Advance counters, skip aux symbols
		i += currentSym->NumberOfAuxSymbols ;
		currentSym += currentSym->NumberOfAuxSymbols ;
		currentSym++ ;
	}

	#ifdef DUMPRAM
	DumpSymbols();
	#endif

	// dump symbolic info if found
	if ( fileSymbol )	{
		const char* auxSym = (const char*)(fileSymbol + 1) ;

		if( strcmpi( latestFile, auxSym ) )	{
			strcpy_s( latestFile, auxSym ) ;
			//JAS      dumpBuffer.Printf( "  file: %s\r\n", auxSym ) ;
		}
	} else {
		latestFile[ 0 ] = 0 ;
		//JAS    dumpBuffer.Printf( "  file: unknown\r\n" ) ;
	}

	if ( fnSymbol )	{
		char tmp_name[1024];
		unmangle(tmp_name, GetSymbolName( fnSymbol ) );
		dumpBuffer << "    " << tmp_name << "()";
	} else {
		dumpBuffer << "    <unknown>";
	}
}


PIMAGE_SECTION_HEADER PE_Debug :: SectionHeaderFromName( const char* name )
  {
  PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION( NT_Header ) ;
  for( unsigned i = 0; i < NT_Header->FileHeader.NumberOfSections; i++ )
    {
    if( strnicmp( (const char*)section->Name, name, IMAGE_SIZEOF_SHORT_NAME ) == 0 )
      return( section ) ;
    else
      section++ ;
    }
  return 0;
  }


PIMAGE_COFF_SYMBOLS_HEADER PE_Debug :: GetDebugHeader()
  {
  // Some files have a wrong entry in the COFF header, so
  // first check if the debug info exists at all
  if( NT_Header->FileHeader.PointerToSymbolTable == 0 )
    return( 0 ) ;
  DWORD debugDirRVA = NT_Header->OptionalHeader.
                      DataDirectory[ IMAGE_DIRECTORY_ENTRY_DEBUG ].
                      VirtualAddress;
  if( debugDirRVA == 0 )
    return( 0 ) ;

  // The following values must be calculated differently for MS/Borland files
  PIMAGE_DEBUG_DIRECTORY debugDir ;
  DWORD size ;

  // Borland files have the debug directory at the beginning of a .debug section
  PIMAGE_SECTION_HEADER debugHeader = SectionHeaderFromName( ".debug" ) ;
  if( debugHeader && debugHeader->VirtualAddress == debugDirRVA )
    {
    debugDir = (PIMAGE_DEBUG_DIRECTORY)(debugHeader->PointerToRawData + (DWORD)fileBase) ;
    size = NT_Header->OptionalHeader.
           DataDirectory[ IMAGE_DIRECTORY_ENTRY_DEBUG ].Size *
           sizeof( IMAGE_DEBUG_DIRECTORY ) ;
    }
  else
  // Microsoft debug directory is in the .rdata section
    {
    debugHeader = SectionHeaderFromName( ".rdata" ) ;
    if( debugHeader == 0 )
      return( 0 ) ;
    size = NT_Header->OptionalHeader.
           DataDirectory[ IMAGE_DIRECTORY_ENTRY_DEBUG ].Size ;
    DWORD offsetInto_rdata = debugDirRVA - debugHeader->VirtualAddress ;
    debugDir = BasedPtr( PIMAGE_DEBUG_DIRECTORY, fileBase,
                         debugHeader->PointerToRawData + offsetInto_rdata ) ;
    }

  // look for COFF debug info
  DWORD debugFormats = size / sizeof( IMAGE_DEBUG_DIRECTORY ) ;
  for( DWORD i = 0; i < debugFormats; i++ )
    {
    if( debugDir->Type == IMAGE_DEBUG_TYPE_COFF )
      return( (PIMAGE_COFF_SYMBOLS_HEADER)((DWORD)fileBase + debugDir->PointerToRawData) ) ;
    else
      debugDir++ ;
    }
  return( NULL ) ;
  }


void PE_Debug :: FindDebugInfo()
  {
  ClearDebugPtrs() ;
  // Put everything into a try/catch in case the file has wrong fields
  try
    {
    // Verify that fileBase is a valid pointer to a DOS header
    if( fileBase->e_magic == IMAGE_DOS_SIGNATURE )
      {
      // Get a pointer to the PE header
      NT_Header = BasedPtr( PIMAGE_NT_HEADERS, fileBase, fileBase->e_lfanew ) ;
      // Verify that NT_Header is a valid pointer to a NT header
      if( NT_Header->Signature == IMAGE_NT_SIGNATURE )
        {
        // Get a pointer to the debug header if any
        COFFDebugInfo = GetDebugHeader() ;
        // Get a pointer to the symbol table and retrieve the number of symbols
        if( NT_Header->FileHeader.PointerToSymbolTable )
          COFFSymbolTable =
            BasedPtr( PIMAGE_SYMBOL, fileBase, NT_Header->FileHeader.PointerToSymbolTable ) ;
        COFFSymbolCount = NT_Header->FileHeader.NumberOfSymbols ;
        // The string table starts right after the symbol table
        stringTable = (const char*)(COFFSymbolTable + COFFSymbolCount) ;
        }
      }
    }
  catch( ... )
    {
    // Header wrong, do nothing
    }
  }


void PE_Debug :: MapFileInMemory( const char* module )
  {
  ClearFileCache() ;
  hFile = CreateFile( module, GENERIC_READ, FILE_SHARE_READ, NULL,
                             OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0 ) ;
  if( hFile != INVALID_HANDLE_VALUE )
    {
    hFileMapping = CreateFileMapping( hFile, NULL, PAGE_READONLY, 0, 0, NULL ) ;
    if( hFileMapping != 0 )
      fileBase = (PIMAGE_DOS_HEADER)MapViewOfFile( hFileMapping, FILE_MAP_READ, 0, 0, 0 ) ;
    }
  // NB: open files/mapping are closed later in ClearFileCache
  }


int PE_Debug::DumpDebugInfo(std::ostream& dumpBuffer, const BYTE* caller, HINSTANCE hInstance )
{
	// Avoid to open, map and looking for debug header/symbol table
	// by caching the latest and comparing the actual module with
	// the latest one.
	static char module[ MAX_MODULENAME_LEN ] ;
	GetModuleFileName( hInstance, module, MAX_MODULENAME_LEN ) ;

	// New module
	if( strcmpi( latestModule, module ) )	{
		strcpy_s( latestModule, module );
		//JAS    dumpBuffer.Printf( "Module: %s\r\n", module );
		MapFileInMemory( module );
		FindDebugInfo();
	}

	char pretty_module[1024];

	strcpy_s( pretty_module, module );
	char *p = pretty_module+strlen(pretty_module)-1;
	// Move p to point to first letter of EXE filename
	while( (*p!='\\') && (*p!='/') && (*p!=':') )
		p--;
	p++;
	if ( strlen(p) < 1 ) {
		strcpy_s( pretty_module, "<unknown>" );
	} else {
		memmove( pretty_module, p, strlen(p)+1 );
	}

	if ( fileBase )	{
		// Put everything into a try/catch in case the file has wrong fields
		try	{
			DWORD relativeAddress = caller - (BYTE*)hInstance ;
			// Dump symbolic information and line number if available
			if( COFFSymbolCount != 0 && COFFSymbolTable != NULL )	{
				DumpSymbolInfo( dumpBuffer, relativeAddress ) ;
				if( COFFDebugInfo )
					DumpLineNumber( dumpBuffer, relativeAddress ) ;
				return 1;
			} else {
				//dumpBuffer.Printf( "Call stack is unavailable, because there is\r\nno COFF debugging info in this module.\r\n" ) ;
				//JAS dumpBuffer.Printf( "  no debug information\r\n" ) ;
				dumpBuffer << "    " << pretty_module << " " << std::setprecision(8)
					<< std::hex << (void*)caller << std::resetiosflags(std::ios::showbase) << "()\n";
				return 0;
			}
		} catch( ... )	{
			// Header wrong, do nothing
			return 0;
      }
	} else	{
		dumpBuffer << "    " << pretty_module << " " << std::setprecision(8) <<
			std::hex << (void*)caller << std::resetiosflags(std::ios::showbase) << "()\n";
		//JAS dumpBuffer.Printf( "  module not accessible\r\n" ) ;
		//JAS dumpBuffer.Printf( "    address: %8X\r\n", caller ) ;
		return 0;
	}

	Int3();
}

void DumpCallsStack( std::ostream& dumpBuffer )
{
	static PE_Debug PE_debug ;

	// The structure of the stack frames is the following:
	// EBP -> parent stack frame EBP
	//        return address for this call ( = caller )
	// The chain can be navigated iteratively, after the
	// initial value of EBP is loaded from the register
	DWORD parentEBP, retval;
	MEMORY_BASIC_INFORMATION mbi ;
	HINSTANCE hInstance;

	int depth = 0;

	__asm MOV parentEBP, EBP

	do	{
		depth++;
		if ( depth > 16 )
			break;

		if ( (parentEBP & 3) || IsBadReadPtr((DWORD*)parentEBP, sizeof(DWORD)) )	{
			break;
		}
		parentEBP = *(DWORD*)parentEBP ;

		BYTE **NextCaller = ((BYTE**)parentEBP + 1);

		if (IsBadReadPtr(NextCaller, sizeof(BYTE *)))	{
			break;
		}

		BYTE* caller = *NextCaller;		// Error sometimes!!!

		// Skip the first EBP as it points to AssertionFailed, which is
		// uninteresting for the user

		if ( depth > 1 )	{

			// Get the instance handle of the module where caller belongs to
			retval = VirtualQuery( caller, &mbi, sizeof( mbi ) ) ;

			// The instance handle is equal to the allocation base in Win32
			hInstance = (HINSTANCE)mbi.AllocationBase ;

			if( ( retval == sizeof( mbi ) ) && hInstance )	{
				if ( !PE_debug.DumpDebugInfo( dumpBuffer, caller, hInstance ) )	{
					//break;
				}
			} else {
				break ; // End of the call chain
			}
		}
	}  while( TRUE ) ;

	PE_debug.ClearReport() ;  // Prepare for future calls
}

#endif	//SHOW_CALL_STACK

SCP_string dump_stacktrace()
{

#if defined( SHOW_CALL_STACK ) && defined( PDB_DEBUGGING )
	/* Dump the callstack */
	SCP_DebugCallStack callStack;
	SCP_DumpStack(dynamic_cast< SCP_IDumpHandler* >(&callStack));

	return callStack.DumpToString();
#elif defined( SHOW_CALL_STACK )
	SCP_stringstream stream;

	DumpCallsStack(stream);

	return stream.str();
#else
	return "No stacktrace available!";
#endif
}
