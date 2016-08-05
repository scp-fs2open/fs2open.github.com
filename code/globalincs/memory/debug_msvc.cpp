
// Only compile in debug builds
#ifndef NDEBUG

#include "debug.h"

#include "globalincs/pstypes.h"

#include <crtdbg.h>

namespace
{
	const int UNTRACKED_SUBTYPE = 5;
	const int UNTRACKED_BLOCK = _CLIENT_BLOCK | (UNTRACKED_SUBTYPE << 16);
	
	// From https://msdn.microsoft.com/en-us/library/974tc9t1.aspx
	// IMPORTANT: For some reason the MS documentation is wrong, the nBlockUse and nDataSize fields are switched
	typedef struct _CrtMemBlockHeader
	{
		// Pointer to the block allocated just before this one:
		struct _CrtMemBlockHeader *pBlockHeaderNext;
		// Pointer to the block allocated just after this one:
		struct _CrtMemBlockHeader *pBlockHeaderPrev;
		char *szFileName;    // File name
		int nLine;           // Line number
#if SCP_COMPILER_VERSION_MAJOR >= 19
		// m!m - I am assuming here that the change of the block header got introduced in my version of VS
		// If memory allocation seems off, check here
		int nBlockUse;       // Type of block
		size_t nDataSize;    // Size of user block
#else
		size_t nDataSize;    // Size of user block
		int nBlockUse;       // Type of block
#endif
		long lRequest;       // Allocation number
							 // Buffer just before (lower than) the user's memory:
		unsigned char gap[4];
	} _CrtMemBlockHeader;

	_CrtMemBlockHeader* get_crt_header(void* ptr)
	{
		return reinterpret_cast<_CrtMemBlockHeader*>(ptr) - 1;
	}

	int allocation_hook(int allocType, void *userData, size_t size, int blockType,
		long requestNumber, const unsigned char *filename, int lineNumber)
	{
		// Ignore CRT blocks
		if (blockType == _CRT_BLOCK)
			return 1;

		// Don't record blocks allocated using the untracked_* versions
		if (blockType == UNTRACKED_BLOCK)
			return 1;
		
		switch (allocType)
		{
		case _HOOK_ALLOC:
		{
			memory::register_malloc(size, reinterpret_cast<const char*>(filename), lineNumber);
			break;
		}
		case _HOOK_FREE:
		{
			auto header = get_crt_header(userData);
			memory::unregister_malloc(header->nDataSize, header->szFileName, header->nLine);
			break;
		}
		case _HOOK_REALLOC:
		{
			if (userData == nullptr)
			{
				memory::register_malloc(size, reinterpret_cast<const char*>(filename), lineNumber);
			}
			else
			{
				auto header = get_crt_header(userData);

				memory::unregister_malloc(header->nDataSize, header->szFileName, header->nLine);
				memory::register_malloc(size, reinterpret_cast<const char*>(filename), lineNumber);
			}
			break;
		}
		default:
			break;
		}

		return 1;
	}
}

namespace memory
{
	void debug_init()
	{
		_CrtSetAllocHook(allocation_hook);
	}

	void debug_exit()
	{
		_CrtDumpMemoryLeaks();
	}
}

void *_vm_malloc(size_t size, const memory::quiet_alloc_t &, const char *filename, int line)
{
	auto ptr = _malloc_dbg(size, _NORMAL_BLOCK, filename, line);

	if (ptr == nullptr)
	{
		mprintf(("Malloc failed!!!!!!!!!!!!!!!!!!!\n"));
	}

	return ptr;
}

void _vm_free(void *ptr, const char*, int)
{
	_free_dbg(ptr, _NORMAL_BLOCK);
}

void *_vm_realloc(void *ptr, size_t size, const memory::quiet_alloc_t &, const char *filename, int line)
{
	auto ret_ptr = _realloc_dbg(ptr, size, _NORMAL_BLOCK, filename, line);

	if (ret_ptr == nullptr)
	{
		mprintf(("Malloc failed!!!!!!!!!!!!!!!!!!!\n"));
	}

	return ret_ptr;
}

void* malloc_untracked(size_t size)
{
	return _malloc_dbg(size, UNTRACKED_BLOCK, nullptr, -1);
}

void free_untracked(void* ptr)
{
	_free_dbg(ptr, UNTRACKED_BLOCK);
}

#endif
