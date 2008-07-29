#include "ai/aibig.h"
#include "ai/aigoals.h"
#include "ai/aiinternal.h"

#ifdef _WIN32

#include <Windows.h>
#define dlopen(x,y) LoadLibrary(x)
#define dlsym GetProcAddress
#define dlclose FreeLibrary
static HINSTANCE handle = NULL;

#else

#include <dlfcn.h>
static void* handle = NULL;

#endif

typedef void (*ai_module_init_function)(void);

int ai_module_dlopen(char* dll)
{
	char buf[PATH_MAX+1];

#ifdef _WIN32
	snprintf(buf, PATH_MAX, "%s.dll", dll);
#else
	snprintf(buf, PATH_MAX, "%s.so", dll);
#endif
	
	ai_module_init_function ai_module_init_func;

	handle = dlopen(buf, RTLD_NOW);

	if (handle == NULL)
		return -1;
	
	ai_module_init_func=(ai_module_init_function)dlsym(handle, "ai_module_init");
	if (ai_module_init_func == NULL)
		return -2;

	ai_module_init_func();

	return 0;	
}

void ai_module_dlclose(void)
{
	if (handle != NULL)
		dlclose(handle);
}

// This inits or resets the AI code tables
void ai_module_init()
{
	ai_module_dlclose();
	
	// First Reset values
	aibig_table = &AIBigDefaultTable;
	aicode_table = &AICodeDefaultTable;
	aigoal_table = &AIGoalDefaultTable;
	aiturret_table = &AITurretDefaultTable;
	
	if (The_mission.ai_profile == NULL)
		return;

	if (strncmp(The_mission.ai_profile->ai_module, "retail", 6) != 0)
	{
		if (ai_module_dlopen(The_mission.ai_profile->ai_module) != 0)
		{
			char err[1024];
			
			snprintf(err, sizeof(err), "AI_module_init(%s) failed: %s\n", The_mission.ai_profile->ai_module, dlerror());
			
			mprintf(err);
		}
	}
}
