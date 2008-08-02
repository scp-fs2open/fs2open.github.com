/**
 * @file aimodular.cpp
 * @brief Modular loading of new AI code based on name in AI Profile.
 *
 * The ai_module_init function should be called by ai_init.
 *
 * @author Fabian Franz <scp@fabian-franz.de>
 * 
 */
#include "ai/aibig.h"
#include "ai/aigoals.h"
#include "ai/aiinternal.h"

#ifdef _WIN32

#include <Windows.h>
/** @brief Wrapper for the dlopen functions for Windows */
#define dlopen(x,y) ((x==NULL)?GetModuleHandle(x):LoadLibrary(x))
#define dlsym GetProcAddress
#define dlclose FreeLibrary
static HINSTANCE handle = NULL; /**< @brief Handle for the loaded library */

#else

#include <dlfcn.h>
static void* handle = NULL; /**< @brief Handle for the loaded library */


#endif

/** @brief ai_module_init_function prototype */
typedef void (*ai_module_init_function)(void);

void ai_module_dlclose(void);

/**
 * @brief Fallback function to load new AI from same process image.
 *
 * This function is used if no dynamic library with the name can be found
 * or it does not contain the correct symbol.
 *
 * @param func_name The symbol to look for in handle
 * @return Returns 0 on success and -1,-2 on errors
 * 
 */
int ai_module_dlopen_process(char* func_name)
{
	ai_module_init_function ai_module_init_func;

	// Maybe close handle first
	ai_module_dlclose();
	
	handle = dlopen(NULL, RTLD_NOW);
	fprintf(stderr, "handle = %p\n", handle);

	if (handle == NULL)
		return -1;
	
	ai_module_init_func=(ai_module_init_function)dlsym(handle, func_name);
	
	fprintf(stderr, "ai_func = %p from %s\n", ai_module_init_func, func_name);

#ifdef _WIN32
	// Under WIN32 GetModuleHandle is not 
	// incrementing the reference counter
	// so we need to "free" the handle directly

	handle=NULL;
#endif
	
	if (ai_module_init_func == NULL)
		return -2;

	ai_module_init_func();

	return 0;
}

/**
 * @brief Loads new AI from dynamic library
 *
 * This function looks for dllname.dll or dllname.so
 * tries to load it into memory and if it finds the 
 * dllname_ai_module_init function it starts it.
 *
 * If it does not find the library or symbol it falls 
 * back to ai_module_dlopen_process.
 *
 * @param dllname The name of the library to load
 * @return Returns 0 on success and -1,-2 on errors
 * 
 */

int ai_module_dlopen(char* dllname)
{
	char buf[PATH_MAX+1];
	char init_func_name[PATH_MAX+1];
	ai_module_init_function ai_module_init_func;

#ifdef _WIN32
	snprintf(buf, PATH_MAX, "%s.dll", dllname);
#else
	snprintf(buf, PATH_MAX, "%s.so", dllname);
#endif
	snprintf(init_func_name, PATH_MAX, "%s_ai_module_init", dllname);
	
	handle = dlopen(buf, RTLD_NOW);
	fprintf(stderr, "handle = %p\n", handle);

	if (handle == NULL)
		return ai_module_dlopen_process(init_func_name);

	ai_module_init_func=(ai_module_init_function)dlsym(handle, init_func_name);

	fprintf(stderr, "ai_func = %p from %s\n", ai_module_init_func, init_func_name);
	if (ai_module_init_func == NULL)
		return ai_module_dlopen_process(init_func_name);

	ai_module_init_func();

	return 0;	
}

/**
 * @brief Closes the library handle
 *
 * This function closes the library handle.
 *
 */
void ai_module_dlclose(void)
{
	if (handle != NULL)
	{
		dlclose(handle);
		handle=NULL;
	}
}

/**
 * @brief Inits or resets the AI code tables
 *
 * This function first resets the AI code tables,
 * then closes any open libraries from before.  
 * 
 * Then it ooks if the ai_module is not "retail"
 * and if that is the case it loads a library with this name.
 * 
 */
void ai_module_init(void)
{
	// First Reset values
	aibig_table = &AIBigDefaultTable;
	aicode_table = &AICodeDefaultTable;
	aigoal_table = &AIGoalDefaultTable;
	aiturret_table = &AITurretDefaultTable;
	
	ai_module_dlclose();
	
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

extern "C" {
	void newai_ai_module_init(void);
}


/**
 * @brief Dummy function 
 *
 * This dummy functions calls all AI implementations 
 * that should be statically included.
 *
 */
void ai_module_dummy()
{
	newai_ai_module_init();
}
