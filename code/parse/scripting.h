#ifndef _SCRIPTING_H
#define _SCRIPTING_H

#include "globalincs/globals.h"
#include "globalincs/pstypes.h"

#include <stdio.h>
#include <vector>

//**********Scripting languages that are possible
#define SC_LUA			(1<<0)
#define SC_PYTHON		(1<<1)

//*************************Scripting structs*************************
#define SCRIPT_END_LIST		NULL

struct image_desc
{
	char fname[MAX_FILENAME_LEN];
	int handle;
};

//**********Main Conditional Hook stuff

#define MAX_HOOK_CONDITIONS	8

//Conditionals
#define CHC_NONE			-1
#define CHC_MISSION			0
#define CHC_SHIP			1
#define CHC_SHIPCLASS		2
#define CHC_SHIPTYPE		3
#define CHC_STATE			4

//Actions
#define CHA_NONE			-1
#define CHA_WARPOUT			0
#define CHA_WARPIN			1
#define CHA_DEATH			2
#define CHA_HOOK			3

struct script_condition
{
	int condition_type;
	union
	{
		char name[NAME_LENGTH];
	} data;

	script_condition(){condition_type = CHC_NONE; memset(data.name, 0, sizeof(data.name));}
};

struct script_action
{
	int action_type;
	script_hook hook;

	script_action(){action_type = CHA_NONE;}
};

class ConditionedHook
{
private:
	std::vector<script_action> Actions;
	script_condition Conditions[MAX_HOOK_CONDITIONS];
public:
	bool AddCondition(script_condition sc);
	bool AddAction(script_action sa);

	bool MaybeRun(class script_state *sys, int action, char format='\0', void *data=NULL, struct object *objp=NULL);
};

//**********Main script_state function
class script_state
{
private:
	char StateName[32];

	int Langs;
	struct lua_State *LuaState;
	const struct script_lua_lib_list *LuaLibs;
	struct PyObject *PyGlb;
	struct PyObject *PyLoc;

	//Utility variables
	std::vector<image_desc> ScriptImages;
	std::vector<ConditionedHook> ConditionalHooks;

private:
	PyObject *GetPyLocals(){return PyLoc;}
	PyObject *GetPyGlobals(){return PyGlb;}

	void ParseChunkSub(int *out_lang, int *out_index, char* debug_str=NULL);
	int RunBytecodeSub(int in_lang, int in_idx, char format='\0', void *data=NULL);

	void SetLuaSession(struct lua_State *L);
	void SetPySession(struct PyObject *loc, struct PyObject *glb);

	void OutputLuaMeta(FILE *fp);

	void Clear();

public:
	//***Init/Deinit
	script_state(char *name);
	script_state& operator=(script_state &in);
	~script_state();

	//***Internal scripting stuff
	int LoadBm(char *name);
	void UnloadImages();

	lua_State *GetLuaSession(){return LuaState;}

	//***Init functions for langs
	int CreateLuaState();
	//int CreatePyState(const script_py_lib_list *libraries);

	//***Get data
	int OutputMeta(char *filename);

	//***Moves data
	//void MoveData(script_state &in);

	//***Variable handling functions
	void SetGlobal(char *name, char format, void *data);
	bool GetGlobal(char *name, char format='\0', void *data=NULL);
	void RemGlobal(char *name);

	//***Hook creation functions
	bool EvalString(char* string, char *format=NULL, void *rtn=NULL, char *debug_str=NULL);
	script_hook ParseChunk(char* debug_str=NULL);
	bool ParseCondition(char* debug_str=NULL);

	//***Hook running functions
	int RunBytecode(script_hook &hd, char format='\0', void *data=NULL);
	int RunCondition(int condition, char format='\0', void *data=NULL, struct object *objp = NULL);
	bool IsOverride(script_hook &hd);
};


//**********Script registration functions
void script_init();

//**********Script globals
extern class script_state Script_system;
extern bool Output_scripting_meta;

//**********Script hook stuff (scripting.tbl)
extern script_hook Script_globalhook;
extern script_hook Script_simulationhook;
extern script_hook Script_hudhook;
extern script_hook Script_splashhook;
extern script_hook Script_gameinithook;

//*************************Conditional scripting*************************

#endif //_SCRIPTING_H
