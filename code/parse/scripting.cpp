#include <stdio.h>
#include <stdarg.h>
#include "parse/scripting.h"
#include "parse/lua.h"
#include "parse/parselo.h"
#include "globalincs/version.h"

//tehe. Declare the main event
script_state Script_system("FS2_Open Scripting");
bool Output_scripting_meta = false;

//*************************Scripting init and handling*************************
std::vector<script_hook> Script_globalhooks;

int script_test(script_state *st)
{
	int rval;
	if ((rval = setjmp(parse_abort)) != 0)
	{
		nprintf(("Warning", "Unable to parse scripting.tbl!  Code = %i.\n", rval));
		return 0;
	}
	else
	{	
		read_file_text("scripting.tbl");
		reset_parse();
	}

	while(optional_string("$Global:"))
	{
		Script_globalhooks.push_back(st->ParseChunk());
	}

	return 1;
}

//Initializes the (global) scripting system, as well as any subsystems.
//script_close is handled by destructors
void script_init (void)
{
#ifdef USE_LUA
	Script_system.CreateLuaState(Lua_libraries);
#endif
	if(Output_scripting_meta)
	{
		Script_system.OutputMeta("scripting.html");
	}
	script_test(&Script_system);
}

//*************************CLASS: script_state*************************
//Most of the icky stuff is here. Lots of #ifdefs
#ifndef USE_LUA
#pragma message( "WARNING: Lua is not compiled in" )
#endif
#ifndef USE_PYTHON
#pragma message("WARNING: Python is not compiled in")
#endif

//returns 0 on failure, 1 on success
int script_state::RunBytecode(script_hook &hd)
{
	if(hd.index >= 0)
	{
		if(hd.language == SC_LUA)
		{
#ifdef USE_LUA
			lua_getref(GetLuaSession(), hd.index);
			lua_pcall(GetLuaSession(), 0, 1, 0);
			return 1;
#endif
		}
		else if(hd.language == SC_PYTHON)
		{
#ifdef USE_PYTHON
			PyObject *chk = PyBytecodeLib.Get(hd.index);
			if(chk != NULL)
			{
				PyEval_EvalCode(PyBytecodeLib.Get(hd.index), GetPyGlobals(), GetPyLocals());
				return 1;
			}
#endif
		}
	}

	return 0;
}

void script_state::Clear()
{
	StateName[0] = '\0';
	Langs = 0;
#ifdef USE_LUA
	//Don't close this yet
	LuaState = NULL;
	LuaLibs = NULL;
#endif
#ifdef USE_PYTHON
	Py_XDECREF(PyGlb);
	PyGlb = NULL;
	Py_XDECREF(PyLoc);
	PyLoc = NULL;
#endif
}

script_state::script_state(char *name)
{
	strncpy(StateName, name, sizeof(StateName)-1);

	Langs = 0;

	LuaState = NULL;
	LuaLibs = NULL;

	PyGlb = NULL;
	PyLoc = NULL;
}

script_state& script_state::operator=(script_state &in)
{
	Error(LOCATION, "SESSION COPY ATTEMPTED");

	return *this;
}

script_state::~script_state()
{
#ifdef USE_LUA
	if(LuaState != NULL) {
		lua_close(LuaState);
	}
#endif
	Clear();
}

void script_state::SetLuaSession(lua_State *L, const struct script_lua_lib_list *libraries)
{
#ifdef USE_LUA
	if(LuaState != NULL)
	{
		lua_close(LuaState);
	}
	LuaState = L;
	LuaLibs = libraries;
	if(LuaState != NULL) {
		Langs |= SC_LUA;
	}
	else if(Langs & SC_LUA) {
		Langs &= ~SC_LUA;
	}
#endif
}

void script_state::SetPySession(PyObject *loc, PyObject *glb)
{
#ifdef USE_PYTHON
	//set variables
	if(PyGlb != NULL)
	{
		Py_XDECREF(PyGlb);
		PyGlb = glb;
	}

	if(PyLoc != NULL)
	{
		Py_XDECREF(PyLoc);
		PyLoc = loc;
	}

	//Add or remove python note
	if(PyGlb != NULL || PyLoc != NULL)
	{
		Langs |= SC_PYTHON;
	}
	else if(Langs & SC_PYTHON)
	{
		Langs &= ~SC_PYTHON;
	}
#endif
}

int script_state::OutputMeta(char *filename)
{
	FILE *fp = fopen(filename,"w");

	if(fp == NULL)
	{
		return 0; 
	}

	fprintf(fp, "<html>\n<head>\n\t<title>Script output: %d.%d.%d (%s)</title>\n</head>\n", FS_VERSION_MAJOR, FS_VERSION_MINOR, FS_VERSION_BUILD, StateName);
	fputs("<body>", fp);
	fprintf(fp,"\t<h1>Script Output - Build %d.%d.%d (%s)</h1>\n", FS_VERSION_MAJOR, FS_VERSION_MINOR, FS_VERSION_BUILD, StateName);
	
	//Scripting languages links
	fputs("<dl>", fp);
	fputs("<dt><b>Scripting languages</b></dt>", fp);
	if(Langs & SC_PYTHON) {
		fputs("<dd><a href=\"#Python\">Python</a></dd>", fp);
	}
	if(Langs & SC_LUA) {
		fputs("<dd><a href=\"#Lua\">Lua</a></dd>", fp);
	}
	fputs("</dl>", fp);

	//Languages
	fputs("<dl>", fp);
	if(Langs & SC_LUA) {
		fputs("<dt><H2><a name=\"#Lua\">Lua</a></H2></dt>", fp);

		fputs("<dd>", fp);
		OutputLuaMeta(fp);
		fputs("</dd>", fp);
	}
	/*
	if(Langs & SC_PYTHON) {
		OutputPyMeta(fp);
	}*/
	fputs("</dl></body></html>", fp);

	fclose(fp);

	return 1;
}

script_hook script_state::ParseChunk(char* debug_str)
{
	static int total_parse_calls = 0;
	char buf[PARSE_BUF_SIZE] = {0};
	char debug_buf[128];
	script_hook rval;

	total_parse_calls++;

	//DANGER! This code means the debug_str must be used only before parsing
	if(!debug_str)
	{
		debug_str = debug_buf;
		sprintf(debug_str, "script_parse() count %d", total_parse_calls);
	}

	if(check_for_string("[["))
	{
		//Lua from file

		//Lua
		rval.language = SC_LUA;

		char *filename = alloc_block("[[", "]]");

#ifdef LUA
		//Load from file
		luaL_loadfile(GetLuaSession(), filename);

		rval.index = luaL_ref(GetLuaSession(), LUA_REGISTRYINDEX);
#endif
		//dealloc
		//commented out b/c it may crash and i don't want to test it now
		//vm_free(filename);
	}
	else if(check_for_string("["))
	{
		//Lua string

		//Assume Lua
		rval.language = SC_LUA;

		//Allocate raw script
		char* raw_lua = alloc_block("[", "]");
#ifdef USE_LUA
		//Load it into a buffer & parse it
		luaL_loadbuffer(GetLuaSession(), raw_lua, strlen(raw_lua), debug_str);

		//Stick it in the registry
		rval.index = luaL_ref(GetLuaSession(), LUA_REGISTRYINDEX);
#endif
		//free the mem
		//WTF THIS CRASHES
		//vm_free(raw_lua);
	}
	else if(check_for_string("{"))
	{
		//Python string

		//Assume python
		rval.language = SC_PYTHON;

		//Get the block
		char* raw_python = alloc_block("{","}");
#ifdef USE_PYTHON
		//Add it to the lib
		rval.index = PyBytecodeLib.Add(PyBytecode(Py_CompileString(raw_python, debug_str, Py_file_input)));
#endif
		//vm_free(raw_python);
	}
	else
	{
		//Python eval
		//Maybe remove this and force brackets, or use custom system?

		//Assume python
		rval.language = SC_PYTHON;

		//Stuff it
		stuff_string(buf, F_RAW, NULL, sizeof(buf)-1);
#ifdef USE_PYTHON
		//Add it to the lib
		rval.index = PyBytecodeLib.Add(PyBytecode(Py_CompileString(buf, debug_str, Py_eval_input));
#endif
	}

	return rval;
}