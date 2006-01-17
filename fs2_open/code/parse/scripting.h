#ifndef _SCRIPTING_H
#define _SCRIPTING_H

#include <stdio.h>
#include <vector>
#include "globalincs/pstypes.h"

//**********Scripting languages that are possible
#define SC_LUA			(1<<0)
#define SC_PYTHON		(1<<1)

//*************************Scripting structs*************************
#define SCRIPT_END_LIST		NULL

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

private:
	PyObject *GetPyLocals(){return PyLoc;}
	PyObject *GetPyGlobals(){return PyGlb;}

	void SetLuaSession(struct lua_State *L);
	void SetPySession(struct PyObject *loc, struct PyObject *glb);

	void OutputLuaMeta(FILE *fp);

	void Clear();

public:
	script_state(char *name);
	script_state& operator=(script_state &in);
	~script_state();

	lua_State *GetLuaSession(){return LuaState;}

	//Init functions for langs
	int CreateLuaState();
	//int CreatePyState(const script_py_lib_list *libraries);

	//Get data
	int OutputMeta(char *filename);

	//Moves data
	//void MoveData(script_state &in);

	//Variable handling functions
	void SetGlobal(char *name, char format, void *data);
	bool GetGlobal(char *name, char format='\0', void *data=NULL);
	void RemGlobal(char *name);

	//Hook handling functions
	script_hook ParseChunk(char* debug_str=NULL);
	int RunBytecode(script_hook &hd, char format='\0', void *data=NULL);
};


//**********Script registration functions
void script_init();

//**********Script globals
extern class script_state Script_system;
extern bool Output_scripting_meta;

//**********Script hook stuff (scripting.tbl)
extern script_hook Script_globalhook;
extern script_hook Script_hudhook;
extern script_hook Script_splashhook;
extern script_hook Script_gameinithook;

#endif //_SCRIPTING_H
