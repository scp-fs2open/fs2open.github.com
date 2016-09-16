
#include "globalincs/pstypes.h"

namespace memory {
const quiet_alloc_t quiet_alloc;
void out_of_memory() {
	mprintf(("Memory allocation failed!!!!!!!!!!!!!!!!!!!\n"));
	Error(LOCATION, "Out of memory.  Try closing down other applications, increasing your\n"
		"virtual memory size, or installing more physical RAM.\n");
}
}
