#ifndef _SCRIPTING_H
#define _SCRIPTING_H

#include <stdio.h>
#include <vector>

//**********Scripting languages that are possible
#define SC_LUA			(1<<0)
#define SC_PYTHON		(1<<1)

//*************************Scripting structs*************************
typedef struct script_hook
{
	int language;
	int index;

	script_hook(){language=0;index=-1;}
}script_hook;

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
	lua_State *GetLuaSession(){return LuaState;}
	PyObject *GetPyLocals(){return PyLoc;}
	PyObject *GetPyGlobals(){return PyGlb;}

	void SetLuaSession(struct lua_State *L, const struct script_lua_lib_list *libraries);
	void SetPySession(struct PyObject *loc, struct PyObject *glb);

	void OutputLuaMeta(FILE *fp);

	void Clear();

public:
	script_state(char *name);
	script_state& operator=(script_state &in);
	~script_state();

	//Init functions for langs
	int CreateLuaState(const struct script_lua_lib_list *libraries);
	//int CreatePyState(const script_py_lib_list *libraries);

	//Get data
	int OutputMeta(char *filename);

	//Moves data
	//void MoveData(script_state &in);

	//Hook handling functions
	script_hook ParseChunk(char* debug_str=NULL);
	int RunBytecode(script_hook &hd);
};


//**********Script registration functions
void script_init();

//**********Script globals
extern class script_state Script_system;
extern bool Output_scripting_meta;

//**********Script hook stuff (scripting.tbl)
extern std::vector<script_hook> Script_globalhooks;

#endif //_SCRIPTING_H
