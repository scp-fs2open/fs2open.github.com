#include "parse/scripting.h"
#ifdef USE_LUA
#include "parse/lua.h"
#include "graphics/2d.h"
#include "ship/ship.h"
#include "ship/shipfx.h"
#include "io/key.h"
#include "io/mouse.h"
#include "gamesequence/gamesequence.h"
#include "globalincs/pstypes.h"
#include "freespace2/freespace.h"
#include "lighting/lighting.h"
#include "render/3dinternal.h"
#include "cmdline/cmdline.h"
#include "playerman/player.h"
#include "mission/missioncampaign.h"
#include "mission/missiongoals.h"
#include "mission/missionload.h"
#include "freespace2/freespace.h"
#include "weapon/weapon.h"
#include "parse/parselo.h"
#include "render/3d.h"

//*************************Lua funcs*************************
int script_remove_lib(lua_State *L, char *name);

//*************************Lua globals*************************
std::vector<lua_lib_h> lua_Libraries;
std::vector<lua_lib_h> lua_Objects;

//*************************Lua helpers*************************
//Function macro
//This is what you call to make new functions
#define LUA_FUNC(name, objlib, args, retvals, desc)	\
	static int lua_##objlib##_##name(lua_State *L);	\
	lua_func_h lua_##objlib##_##name##_h(#name, lua_##objlib##_##name, objlib, args, retvals, desc);	\
	static int lua_##objlib##_##name(lua_State *L)

//Use this to handle forms of type vec.x and vec['x']. Basically an indexer for a specific variable.
//Format string should be "o*%", where * is indexing value, and % is the value to set to when LUA_SETTTING_VAR is set
#define LUA_VAR(name, objlib, type, desc)			\
	static int lua_##objlib##_var##name(lua_State *L);	\
	lua_var_h lua_##objlib##_var##name##_h(#name, lua_##objlib##_var##name, objlib, false, type, desc);	\
	static int lua_##objlib##_var##name(lua_State *L)
//WMC - Doesn't work
/*
#define LUA_ARRAY(name, objlib, type, desc)			\
	static int lua_##objlib##_var##name(lua_State *L);	\
	lua_var_h lua_##objlib##_var##name##_h(#name, lua_##objlib##_var##name, objlib, true, type, desc);	\
	static int lua_##objlib##_var##name(lua_State *L)
*/

//Use this with objects to deal with forms such as vec.x, vec['x'], vec[0]
//Format string should be "o*%", where * is indexing value, and % is the value to set to when LUA_SETTTING_VAR is set
#define LUA_INDEXER(objlib, desc)			\
	static int lua_##objlib##___indexer(lua_State *L);	\
	lua_indexer_h lua_##objlib##___indexer_h(lua_##objlib##___indexer, objlib, desc);	\
	static int lua_##objlib##___indexer(lua_State *L)

//Checks to determine whether LUA_VAR or LUA_INDEXER should set the variable
#define LUA_SETTING_VAR	lua_toboolean(L,lua_upvalueindex(1))

struct string_conv {
	char *src;
	char *dest;
};

//*************************Lua operators*************************
//These are the various types of operators you can
//set in Lua. Use these as function name to activate.
//
//Format string should be "*o" or "o*", where "*" is the type of
//variable you want to deal with.
//The order varies with order of variables
string_conv lua_Operators[] = {
	{"__add",		"+"},			//var +  obj
	{"__sub",		"-"},			//var -  obj
	{"__mult",		"*"},			//var *  obj
	{"__div",		"/"},			//var /  obj
	{"__pow",		"^"},			//var ^  obj
	{"__unm",		"~"},			//var ~  obj
	{"__concat"		".."},			//var .. obj	NOTE: Automatically added (calls tostring if possible), add yourself to override
	{"__eq",		"=="},			//var == obj
	{"__lt",		"<"},			//var <  obj
	{"__le",		"<="},			//var <= obj
	{"__tostring",	"(string)"},	//(String) obj
	{"__call",		"?"},			//*shrug*
	//WMC - This is NOT a Lua type, but for the LUA_INDEXER define
	{"__indexer",	"[]"},			//obj[var]
};

int lua_Num_operators = sizeof(lua_Operators)/sizeof(string_conv);

//*************************Lua return values*************************
#define LUA_RETURN_NIL		0
#define LUA_RETURN_OBJECT		1
#define LUA_RETURN_TRUE		lua_set_args(L, "b", true)
#define LUA_RETURN_FALSE	lua_set_args(L, "b", true)

//*************************General Functions*************************
//WMC - This doesn't work.
int script_remove_lib(lua_State *L, char *name)
{
	lua_pushstring(L, name);
	lua_gettable(L, LUA_GLOBALSINDEX);
	if(lua_istable(L, -1))
	{
		lua_pop(L, -1);
		return 1;
	}

	return 0;
}

//WMC - Spits out the current Lua stack to "stackdump"
//This includes variable values, but not names
void lua_stackdump(lua_State *L, char *stackdump)
{
	char buf[512];
	int stacksize = lua_gettop(L);

	//Lua temps
	double d;
	int b;
	char *s;
//	void *v;
//	lua_State *ls;
	for(int argnum = 1; argnum <= stacksize; argnum++)
	{
		int type = lua_type(L, argnum);
		sprintf(buf, "\r\n%d: ", argnum);
		strcat(stackdump, buf);
		switch(type)
		{
			case LUA_TNIL:
				strcat(stackdump, "NIL");
				break;
			case LUA_TNUMBER:
				d = lua_tonumber(L, argnum);
				sprintf(buf, "Number [%f]",d);
				strcat(stackdump, buf);
				break;
			case LUA_TBOOLEAN:
				b = lua_toboolean(L, argnum);
				sprintf(buf, "Boolean [%d]",b);
				strcat(stackdump, buf);
				break;
			case LUA_TSTRING:
				s = (char *)lua_tostring(L, argnum);
				sprintf(buf, "String [%s]",s);
				strcat(stackdump, buf);
				break;
			case LUA_TTABLE:
				strcat(stackdump, "Table");
				break;
			case LUA_TFUNCTION:
				strcat(stackdump, "Function");
				break;
			case LUA_TUSERDATA:
				//v = lua_touserdata(L, argnum);
				lua_getmetatable(L, argnum);
				lua_rawget(L, LUA_REGISTRYINDEX);
				s = (char *)lua_tostring(L, -1);
				lua_pop(L, 1);
				sprintf(buf, "Userdata [%s]", s);
				strcat(stackdump, buf);
				break;
			case LUA_TTHREAD:
				//ls = lua_tothread(L, argnum);
				sprintf(buf, "Thread");
				strcat(stackdump, buf);
				break;
			case LUA_TLIGHTUSERDATA:
				//v = lua_touserdata(L, argnum);
				sprintf(buf, "Light userdata");
				strcat(stackdump, buf);
				break;
			default:
				sprintf(buf, "<UNKNOWN>: %s (%f) (%s)", lua_typename(L, type), lua_tonumber(L, argnum), lua_tostring(L, argnum));
				strcat(stackdump, buf);
				break;
		}
	}
}

//WMC - Gets type of object
char *lua_get_type_string(lua_State *L, int argnum)
{
	int type = lua_type(L, argnum);
	switch(type)
	{
		case LUA_TNIL:
			return "Nil";
		case LUA_TNUMBER:
			return "Number";
		case LUA_TBOOLEAN:
			return "Boolean";
		case LUA_TSTRING:
			return "String";
		case LUA_TTABLE:
			return "Table";
		case LUA_TFUNCTION:
			return "Function";
		case LUA_TUSERDATA:
			return "Userdata";
		case LUA_TTHREAD:
			return "Thread";
		case LUA_TLIGHTUSERDATA:
			return "Light Userdata";
		default:
			return "Unknown";
	}
}

int lua_get_object(char *name)
{
	for(int i = 0; i < (int)lua_Objects.size(); i++)
	{
		if(!stricmp(lua_Objects[i].Name, name))
			return i;
	}

	return -1;
}

//WMC - hack to skip X number of arguments on the stack
//Lets me use lua_get_args for global hook return values
int Lua_get_args_skip = 0;

//lua_get_args(state, arguments, variables)
//----------------------------------------------
//based on "Programming in Lua"
//
//Parses arguments from string to variables given
//a '|' divides required and optional arguments.
//Returns 0 if a required argument is invalid,
//or there are too few arguments actually passed
//
//NOTE: This function essentially takes objects
//from the stack in series, so it can easily be used
//to get the return values from a chunk of Lua code
//after it has been executed. See RunByteCode()
int lua_get_args(lua_State *L, char *fmt, ...)
{
	//Check that we have all the arguments that we need
	//If we don't, return 0
	int needed_args = strlen(fmt);
	int total_args = lua_gettop(L) - Lua_get_args_skip;

	if(strchr(fmt, '|') != NULL) {
		needed_args = strchr(fmt, '|') - fmt;
	}

	if(total_args < needed_args) {
		LuaError(L, "Not enough arguments for function - need %d, had %d. If you are using objects or handles, make sure that you are using \":\" to access member functions, rather than \".\"", needed_args, total_args);
		return 0;
	}

	char funcname[128];
#ifndef NDEBUG
	lua_Debug ar;
	lua_getstack(L, 0, &ar);
	lua_getinfo(L, "nl", &ar);
	strcpy(funcname, "");
	if(ar.name != NULL) {
		strcat(funcname, ar.name);
	}
	if(ar.currentline > -1) {
		char buf[8];
		itoa(ar.currentline, buf, 10);
		strcat(funcname, " (Line ");
		strcat(funcname, buf);
		strcat(funcname, ")");
	}
	if(!strlen(funcname)) {
		//WMC - Try and get at function name from upvalues
		if(lua_type(L, lua_upvalueindex(1)) == LUA_TSTRING)
			strcpy(funcname, lua_tostring(L, lua_upvalueindex(1)));
		else if(lua_type(L, lua_upvalueindex(2)) == LUA_TSTRING)
			strcpy(funcname, lua_tostring(L, lua_upvalueindex(2)));

		//WMC - Totally unknown function
		if(!strlen(funcname)) {
			strcpy(funcname, "<UNKNOWN>");
		}
	}
#endif

	//Start throught
	va_list vl;
	int nargs;

	//Are we parsing optional args yet?
	bool optional_args = false;

	va_start(vl, fmt);
	nargs = 1 + Lua_get_args_skip;
	total_args += Lua_get_args_skip;
	while(*fmt && nargs <= total_args)
	{
		//Skip functions; I assume these are being used to return args
		while(lua_type(L, nargs) == LUA_TFUNCTION && nargs <= total_args)
			nargs++;

		switch(*fmt++)
		{
			case 'b':
				if(lua_isboolean(L, nargs)) {
					*va_arg(vl, bool*) = lua_toboolean(L, nargs) > 0 ? true : false;
				} else {
					LuaError(L, "%s: Argument %d is an invalid type '%s'; boolean expected", funcname, nargs, lua_get_type_string(L, nargs));
					if(!optional_args) return 0;
				}
				break;
			case 'd':
				if(lua_isnumber(L, nargs)) {
					*va_arg(vl, double*) = (double)lua_tonumber(L, nargs);
				} else {
					LuaError(L, "%s: Argument %d is an invalid type '%s'; number expected", funcname, nargs, lua_get_type_string(L, nargs));
					if(!optional_args) return 0;
				}
				break;
			case 'f':
				if(lua_isnumber(L, nargs)) {
					*va_arg(vl, float*) = (float)lua_tonumber(L, nargs);
				} else {
					LuaError(L, "%s: Argument %d is an invalid type '%s'; number expected", funcname, nargs, lua_get_type_string(L, nargs));
					if(!optional_args) return 0;
				}
				break;
			case 'i':
				if(lua_isnumber(L, nargs)) {
					*va_arg(vl, int*) = (int)lua_tonumber(L, nargs);
				} else {
					LuaError(L, "%s: Argument %d is an invalid type '%s'; number expected", funcname, nargs, lua_get_type_string(L, nargs));
					if(!optional_args) return 0;
				}
				break;
			case 's':
				if(lua_isstring(L, nargs)) {
					*va_arg(vl, const char **) = lua_tostring(L, nargs);
				} else {
					LuaError(L, "%s: Argument %d is an invalid type '%s'; string expected", funcname, nargs, lua_get_type_string(L, nargs));
					if(!optional_args) return 0;
				}
				break;
			case 'o':
				if(lua_isuserdata(L, nargs))
				{
					script_lua_odata od = va_arg(vl, script_lua_odata);
					lua_getmetatable(L, nargs);
					lua_rawget(L, LUA_REGISTRYINDEX);
					char *s = (char *)lua_tostring(L, -1);
					if(stricmp(s, lua_Objects[od.meta].Name)) {
						int idx = lua_get_object(s);
						if(idx < 0 || (lua_Objects[idx].Derivator != od.meta)) {
							LuaError(L, "%s: Argument %d is the wrong type of userdata; %s given, but %s expected", funcname, nargs, s, lua_Objects[od.meta].Name);
						}
					}
					if(s) lua_pop(L, 1);
					if(od.size != ODATA_PTR_SIZE)
					{
						memcpy(od.buf, lua_touserdata(L, nargs), od.size);
					} else {
						(*(void**)od.buf) = lua_touserdata(L, nargs);
					}
				}
				else
				{
					LuaError(L, "%s: Argument %d is an invalid type '%s'; type '%s' expected", funcname, nargs, lua_get_type_string(L, nargs), lua_Objects[va_arg(vl, script_lua_odata).meta].Name);
					if(!optional_args) return 0;
				}
				break;
			case '|':
				nargs--;	//cancel out the nargs++ at the end
				optional_args = true;
				break;
			default:
				Error(LOCATION, "%s: Bad character passed to lua_get_args; (%c)", funcname, *(fmt-1));
				break;
		}
		nargs++;
	}
	va_end(vl);
	return nargs--;
}

//lua_set_args(state, arguments, variables)
//----------------------------------------------
//based on "Programming in Lua"
//
//Takes variables given and pushes them onto the
//Lua stack. Use it to return variables from a
//Lua scripting function.
//
//NOTE: You can also use this to push arguments
//on to the stack in series. See script_state::SetGlobal
int lua_set_args(lua_State *L, char *fmt, ...)
{
	//Start throught
	va_list vl;
	int nargs;

	va_start(vl, fmt);
	nargs = 0;
	while(*fmt != '\0')
	{
		switch(*fmt++)
		{
			case 'b':	//WMC - Bool for GCC (Why...?)
				lua_pushboolean(L, va_arg(vl, int) ? 1 : 0);
				break;
			case 'd':
				lua_pushnumber(L, va_arg(vl, double));
				break;
			case 'f':
				lua_pushnumber(L, va_arg(vl, double));
				break;
			case 'i':
				lua_pushnumber(L, va_arg(vl, int));
				break;
			case 's':
				lua_pushstring(L, va_arg(vl, char *));
				break;
			case 'o':
				{
					//WMC - step by step
					//Copy over objectdata
					script_lua_odata od = va_arg(vl, script_lua_odata);

					//Create new LUA object and get handle
					void *newod = (void*)lua_newuserdata(L, od.size);
					//Create or get object metatable
					luaL_getmetatable(L, lua_Objects[od.meta].Name);
					//Set the metatable for the object
					lua_setmetatable(L, -2);

					//Copy the actual object data to the Lua object
					memcpy(newod, od.buf, od.size);
					break;
				}
			//WMC -  Don't forget to update lua_set_arg
			default:
				Error(LOCATION, "Bad character passed to lua_set_args; (%c)", *fmt);
		}
		nargs++;
	}
	va_end(vl);
	return nargs;
}

int lua_friendly_error(lua_State *L)
{
	LuaError(L);

	//WMC - According to documentation, this will always be the error
	//if error handler is called
	return LUA_ERRRUN;
}

//WMC - Used to automatically use an object's __tostring function to concatenate
int lua_concat_handler(lua_State *L)
{
	const char *s = NULL;
	int objpos = 2;
	if(lua_isstring(L, 1)) {
		s = lua_tostring(L, 1);
	} else {
		s = lua_tostring(L, 2);
		objpos = 1;
	}

	//Get metatable
	lua_getmetatable(L, objpos);

	//Plan for the future - stacksize & error handler
	int stacksize = lua_gettop(L);
	lua_pushcfunction(L, lua_friendly_error);

	//Get tostring function
	lua_pushstring(L, "__tostring");
	lua_rawget(L, -3);

	if(!lua_iscfunction(L, -1)) {
		//Return original string to fail gracefully
		lua_pushstring(L, s);
		return 1;
	}

	//Push the object
	lua_pushvalue(L, objpos);
	if(lua_pcall(L, 1, 1, -3) || lua_gettop(L) - stacksize < 1)
	{
		//Return original string to fail gracefully
		lua_pushstring(L, s);
		return 1;
	}

	//Now take return value and concat
	const char *s2 = lua_tostring(L, -1);
	char *buf = new char[strlen(s) + strlen(s2) + 1];
	strcpy(buf, "");
	//String in back
	if(objpos == 2) {
		strcat(buf, s);
	}

	strcat(buf, s2);

	//Original string in back
	if(objpos == 1) {
		strcat(buf, s);
	}

	lua_pushstring(L, buf);
	delete[] buf;
	return 1;
}

//WMC - Bogus integer used to determine if a variable
//in an object or library is modder-defined or code-defined.
#define INDEX_HANDLER_VAR_TRIGGER	1337

//Depends on one upvalue, a boolean.
//false => __index
//true => __newindex
int lua_index_handler(lua_State *L)
{
	//WMC - We might need this. It's easier to push it now and deal with it later
	lua_pushcfunction(L, lua_friendly_error);
	//WMC - When this function is called, there should be
	//two values on the stack - the object, and the key
	//Everything else, then, is an argument.
	int args_start = 2;
	int args_stop = lua_gettop(L);

	//Get the data at key in metatable
	lua_getmetatable(L, 1);	//Get metatable
	lua_pushvalue(L, 2);	//Get key

	bool using_indexer = lua_type(L, 2) != LUA_TSTRING;
	if(!using_indexer)
	{
		lua_rawget(L, -2);
		if(lua_isnil(L, -1)) {
			using_indexer = true;
		}
	}
	
	//WMC - I tried using lua_isnil here to use the indexer if an
	//unknown string key was passed, but for some reason I got a crash
	//at location 0x00000069 every time lua_pcall was called.
	//Whether or not this is an internal bug, a bug on my part,
	//or some Lua programmer sex joke, I don't know.
	if(!using_indexer)
	{
		lua_rawget(L, -2);
		if(lua_isnil(L, -1)) {
			using_indexer = true;
		}
	}
	//WMC - UPDATE!! This magically fixed itself or something.
	//Persistence pays off.
	//If this bug crops up again, the fix is to comment out the sandwiched
	//if section, and uncomment the lua_rawget(L, -2) in the next !using_indexer
	//if section.
	if(!using_indexer)
	{
		//WMC - 0x00000069 bug fixed itself
		//lua_rawget(L, -2);

		//If it's a function or doesn't exist, we're done
		//If it does exist but doesn't have the bogus value, we're also done. Either
		//the modder messed with the variable or they set it themself.
		if(lua_iscfunction(L, -1) || lua_isnil(L, -1) || lua_tonumber(L, -1) != INDEX_HANDLER_VAR_TRIGGER) {
			//Push another copy of the object data onto the stack to return
			lua_pushvalue(L, 1);
			return 2;	//return object data and function
		}

		//WMC - Apparently we have a variable that the code is responsible for
		lua_pop(L, 1);

		//Allocate enough space for "var" + string
		const char *s = lua_tostring(L, 2);	//WMC - IMPORTANT: Key is now a string.
		char *funcname = new char[lua_strlen(L, 2) + 4];
		strcpy(funcname, "var");
		strcat(funcname, s);

		//Return function (if it's there)
		//Metatable is still there
		lua_pushstring(L, funcname);	//Push key
		lua_rawget(L, -2);
	}
	else
	{
		//Get rid of key on top of stack
		lua_pop(L, 1);

		lua_pushstring(L, "__indexer");
		lua_rawget(L, -2);
	}

	//Set upvalue for function to same upvalue as this
	//This tells it whether it is getting or setting the variable
	lua_pushvalue(L, lua_upvalueindex(1));
	if(lua_setupvalue(L, -2, 1) == NULL) {
		//setupvalue failed for some reason,
		//so pop the bool ourselves.
		Warning(LOCATION, "lua_setupvalue in lua_index_handler failed; get a coder");
		lua_pop(L, 1);
	}

	//*****BEGIN FUNCTION ARGUMENT HANDLING
	//WMC - Pass error handler function
	int passed_args = 1;	//Object data
	//Push another copy of the object data onto the stack to return
	lua_pushvalue(L, 1);

	//WMC - save size of stack, minus arguments
	int old_stacksize = lua_gettop(L) - 2;
	if(using_indexer)
	{
		//Push another copy of key onto stack to return
		lua_pushvalue(L, 2);
		passed_args++;
		//No need to change stacksize, it doesn't know about this
	}

	//Push arguments onto stack.
	for(; args_stop > args_start; args_stop--) {
		lua_pushvalue(L, args_stop);
		passed_args++;
	}

	//Finally, call the function to get/set the variable
	//Use 1 argument for string key, 2 for indexer
	//WMC - Error handler at top of function is last arg
	if(lua_pcall(L, passed_args, LUA_MULTRET, 3) != 0)
	{
		return 0;
	}

	//Return however many things the function call put on the stack
	return lua_gettop(L) - old_stacksize;
}

//*************************Begin non-lowlevel stuff*************************
//If you are a coder who wants to add functionality to Lua, you want to be
//below this point.

//**********CLASS: vector
lua_obj<vec3d> l_Vector("vector", "Vector");

LUA_INDEXER(l_Vector, "Vector component")
{
	vec3d *v3;
	char *s = NULL;
	float newval = 0.0f;
	int numargs = lua_get_args(L, "os|f", l_Vector.GetPtr(&v3), &s, &newval);

	if(!numargs || s[1] != '\0')
		LUA_RETURN_NIL;

	int idx=-1;
	if(s[0]=='x' || s[0] == '1')
		idx = 0;
	else if(s[0]=='y' || s[0] == '2')
		idx = 1;
	else if(s[0]=='z' || s[0] == '3')
		idx = 2;

	if(idx < 0 || idx > 2)
		return LUA_RETURN_NIL;

	if(LUA_SETTING_VAR) {
		v3->a1d[idx] = newval;
	}

	return lua_set_args(L, "f", v3->a1d[idx]);
}

LUA_FUNC(__add, l_Vector, "{Number, Vector}", "Vector", "Adds vector by another vector, or adds all axes by value")
{
	vec3d v3;
	if(lua_isnumber(L, 1) || lua_isnumber(L, 2))
	{
		float f;
		if(lua_isnumber(L, 1) && lua_get_args(L, "fo", &f, l_Vector.Get(&v3))
			|| lua_isnumber(L, 2) && lua_get_args(L, "of", l_Vector.Get(&v3), &f))
		{
			v3.xyz.x += f;
			v3.xyz.y += f;
			v3.xyz.z += f;
		}
	}
	else
	{
		vec3d v3b;
		//WMC - doesn't really matter which is which
		if(lua_get_args(L, "oo", l_Vector.Get(&v3), l_Vector.Get(&v3b)))
		{
			vm_vec_add2(&v3, &v3b);
		}
	}
	return lua_set_args(L, "o", l_Vector.Set(v3));
}

LUA_FUNC(__sub, l_Vector, "{Number, Vector}", "Vector", "Subtracts vector from another vector, or subtracts all axes by value")
{
	vec3d v3;
	if(lua_isnumber(L, 1) || lua_isnumber(L, 2))
	{
		float f;
		if(lua_isnumber(L, 1) && lua_get_args(L, "fo", &f, l_Vector.Get(&v3))
			|| lua_isnumber(L, 2) && lua_get_args(L, "of", l_Vector.Get(&v3), &f))
		{
			v3.xyz.x += f;
			v3.xyz.y += f;
			v3.xyz.z += f;
		}
	}
	else
	{
		vec3d v3b;
		//WMC - doesn't really matter which is which
		if(lua_get_args(L, "oo", l_Vector.Get(&v3), l_Vector.Get(&v3b)))
		{
			vm_vec_sub2(&v3, &v3b);
		}
	}

	return lua_set_args(L, "o", l_Vector.Set(v3));
}

LUA_FUNC(__mult, l_Vector, "Number", "Vector", "Scales vector object (Multiplies all axes by number)")
{
	vec3d v3;
	if(lua_isnumber(L, 1) || lua_isnumber(L, 2))
	{
		float f;
		if(lua_isnumber(L, 1) && lua_get_args(L, "fo", &f, l_Vector.Get(&v3))
			|| lua_isnumber(L, 2) && lua_get_args(L, "of", l_Vector.Get(&v3), &f))
		{
			vm_vec_scale(&v3, f);
		}
	}

	return lua_set_args(L, "o", l_Vector.Set(v3));
}

LUA_FUNC(__div, l_Vector, "Number", "Vector", "Scales vector object (Divide all axes by number)")
{
	vec3d v3;
	if(lua_isnumber(L, 1) || lua_isnumber(L, 2))
	{
		float f;
		if(lua_isnumber(L, 1) && lua_get_args(L, "fo", &f, l_Vector.Get(&v3))
			|| lua_isnumber(L, 2) && lua_get_args(L, "of", l_Vector.Get(&v3), &f))
		{
			vm_vec_scale(&v3, 1.0f/f);
		}
	}

	return lua_set_args(L, "o", l_Vector.Set(v3));
}


LUA_FUNC(__tostring, l_Vector, NULL, "String", "Converts a vector to string with format \"(x,y,z)\"")
{
	vec3d v3;
	if(!lua_get_args(L, "o", l_Vector.Get(&v3)))
		return LUA_RETURN_NIL;

	char buf[32];
	sprintf(buf, "(%f,%f,%f)", v3.xyz.x, v3.xyz.y, v3.xyz.z);

	return lua_set_args(L, "s", buf);
}

LUA_FUNC(getDotProduct, l_Vector, "vector argument", "Dot product (number)", "Returns dot product of vector object with vector argument")
{
	vec3d *v3a, *v3b;
	if(!lua_get_args(L, "oo", l_Vector.GetPtr(&v3a), l_Vector.GetPtr(&v3b)))
		return LUA_RETURN_NIL;

	return lua_set_args(L, "f", vm_vec_dotprod(v3a, v3b));
}

LUA_FUNC(getCrossProduct, l_Vector, "vector argument", "Cross product (number)", "Returns cross product of vector object with vector argument")
{
	vec3d *v3a, *v3b;
	if(!lua_get_args(L, "oo", l_Vector.GetPtr(&v3a), l_Vector.GetPtr(&v3b)))
		return LUA_RETURN_NIL;

	vec3d v3r;
	vm_vec_crossprod(&v3r, v3a, v3b);

	return lua_set_args(L, "o",l_Vector.Set(v3r));
}

LUA_FUNC(getScreenCoords, l_Vector, NULL, "X (number), Y (number), or false if off-screen", "Gets screen cordinates of a vector (presumed in world coordinates)")
{
	vec3d v3;
	if(!lua_get_args(L, "o", l_Vector.Get(&v3)))
		return LUA_RETURN_NIL;

	vertex vtx;
	bool do_g3 = G3_count < 1;
	if(do_g3)
		g3_start_frame(1);
	
	g3_rotate_vertex(&vtx,&v3);
	g3_project_vertex(&vtx);
	gr_unsize_screen_posf( &vtx.sx, &vtx.sy );

	if(do_g3)
		g3_end_frame();

	if(vtx.flags & PF_OVERFLOW)
		return LUA_RETURN_FALSE;

	return lua_set_args(L, "ii", vtx.sx, vtx.sy);
}

//**********CLASS: cmission
lua_obj<int> l_Cmission("cmission", "Campaign mission object");
//WMC - We can get away with a pointer right now, but if it ever goes dynamic, it'd be a prob

int lua_cmission_helper(lua_State *L, int *idx)
{
	*idx = -1;
	if(!lua_get_args(L, "o", idx))
		return 0;

	if(*idx < 0 || *idx > Campaign.num_missions)
		return 0;

	return 1;
}

LUA_FUNC(getName, l_Cmission, NULL, "Mission name", "Gets mission name")
{
	int idx;
	if(!lua_cmission_helper(L, &idx))
		return LUA_RETURN_NIL;

	return lua_set_args(L, "s", Campaign.missions[idx].name);
}

LUA_FUNC(isCompleted, l_Cmission, NULL, "True or false", "Returns true if mission completed, false if not")
{
	int idx;
	if(!lua_cmission_helper(L, &idx))
		return LUA_RETURN_NIL;

	return lua_set_args(L, "b", Campaign.missions[idx].completed ? true : false);
}

LUA_FUNC(getNotes, l_Cmission, NULL, "Mission notes (string), or false if none", "Gets mission notes")
{
	int idx;
	if(!lua_cmission_helper(L, &idx))
		return LUA_RETURN_NIL;

	if(Campaign.missions[idx].notes == NULL)
		return LUA_RETURN_FALSE;

	return lua_set_args(L, "s", Campaign.missions[idx].notes);
}

LUA_FUNC(getMainHallNum, l_Cmission, NULL, "Main hall number", "Gets the main hall number for this mission")
{
	int idx;
	if(!lua_cmission_helper(L, &idx))
		return LUA_RETURN_NIL;

	return lua_set_args(L, "i", Campaign.missions[idx].main_hall);
}

LUA_FUNC(getCutsceneName, l_Cmission, NULL, "Cutscene name, or false if none", "Gets the name of the cutscene for this mission (Usually played before command briefing)")
{
	int idx;
	if(!lua_cmission_helper(L, &idx))
		return LUA_RETURN_NIL;

	if(!strlen(Campaign.missions[idx].briefing_cutscene))
		return LUA_RETURN_FALSE;

	return lua_set_args(L, "s", Campaign.missions[idx].briefing_cutscene);
}

LUA_FUNC(getNumGoals, l_Cmission, NULL, "Number of goals", "Gets the number of goals for this mission")
{
	int idx;
	if(!lua_cmission_helper(L, &idx))
		return LUA_RETURN_NIL;

	return lua_set_args(L, "i", Campaign.missions[idx].num_goals);
}

LUA_FUNC(getGoalName, l_Cmission, "Goal number (Zero-based)", "Name of goal", "Gets the name of the goal")
{
	int idx = -1;
	int gidx = -1;
	if(!lua_get_args(L, "oi", &idx, &gidx))
		return LUA_RETURN_NIL;

	if(idx < 0 || idx > Campaign.num_missions)
		return LUA_RETURN_NIL;

	if(gidx < 0 || gidx > Campaign.missions[idx].num_goals)
		return LUA_RETURN_NIL;

	return lua_set_args(L, "i", Campaign.missions[idx].goals[gidx].name);
}

LUA_FUNC(getGoalStatus, l_Cmission, "Goal number (Zero-based)", "Goal status (string)", "Gets the status of the goal - Failed, Complete, or Incomplete")
{
	int idx = -1;
	int gidx = -1;
	if(!lua_get_args(L, "oi", &idx, &gidx))
		return LUA_RETURN_NIL;

	if(idx < 0 || idx > Campaign.num_missions)
		return LUA_RETURN_NIL;

	if(gidx < 0 || gidx > Campaign.missions[idx].num_goals)
		return LUA_RETURN_NIL;

	char buf[NAME_LENGTH];

	switch( Campaign.missions[idx].goals[gidx].status)
	{
		case GOAL_FAILED:
			strcpy(buf, "Failed");
			break;
		case GOAL_COMPLETE:
			strcpy(buf, "Complete");
			break;
		case GOAL_INCOMPLETE:
			strcpy(buf, "Incomplete");
			break;
		default:
			Int3();		//????
			return LUA_RETURN_FALSE;
	}

	return lua_set_args(L, "s", buf);
}

LUA_FUNC(getNumEvents, l_Cmission, NULL, "Number of events", "Gets the number of events for this mission")
{
	int idx;
	if(!lua_cmission_helper(L, &idx))
		return LUA_RETURN_NIL;

	return lua_set_args(L, "i", Campaign.missions[idx].num_events);
}

LUA_FUNC(getEventName, l_Cmission, "Event number (Zero-based)", "Name of event", "Gets the name of the event")
{
	int idx = -1;
	int eidx = -1;
	if(!lua_get_args(L, "oi", &idx, &eidx))
		return LUA_RETURN_NIL;

	if(idx < 0 || idx > Campaign.num_missions)
		return LUA_RETURN_NIL;

	if(eidx < 0 || eidx > Campaign.missions[idx].num_events)
		return LUA_RETURN_NIL;

	return lua_set_args(L, "i", Campaign.missions[idx].events[eidx].name);
}

LUA_FUNC(getEventStatus, l_Cmission, "Event number (Zero-based)", "Event status (string)", "Gets the status of the event - Failed, Complete, or Incomplete")
{
	int idx = -1;
	int eidx = -1;
	if(!lua_get_args(L, "oi", &idx, &eidx))
		return LUA_RETURN_NIL;

	if(idx < 0 || idx > Campaign.num_missions)
		return LUA_RETURN_NIL;

	if(eidx < 0 || eidx > Campaign.missions[idx].num_events)
		return LUA_RETURN_NIL;

	char buf[NAME_LENGTH];

	switch( Campaign.missions[idx].goals[eidx].status)
	{
		case EVENT_FAILED:
			strcpy(buf, "Failed");
			break;
		case EVENT_SATISFIED:
			strcpy(buf, "Complete");
			break;
		case EVENT_INCOMPLETE:
			strcpy(buf, "Incomplete");
			break;
		default:
			Int3();		//????
			return LUA_RETURN_FALSE;
	}

	return lua_set_args(L, "s", buf);
}

LUA_FUNC(getNumVariables, l_Cmission, NULL, "Number of variables", "Gets the number of saved SEXP variables for this mission")
{
	int idx;
	if(!lua_cmission_helper(L, &idx))
		return LUA_RETURN_NIL;

	return lua_set_args(L, "i", Campaign.missions[idx].num_saved_variables);
}

LUA_FUNC(getVariableName, l_Cmission, "Variable number (Zero-based)", "Variable name", "Gets the name of the variable")
{
	int idx = -1;
	int vidx = -1;
	if(!lua_get_args(L, "oi", &idx, &vidx))
		return LUA_RETURN_NIL;

	if(idx < 0 || idx > Campaign.num_missions)
		return LUA_RETURN_NIL;

	if(vidx < 0 || vidx > Campaign.missions[idx].num_saved_variables)
		return LUA_RETURN_NIL;

	return lua_set_args(L, "i", Campaign.missions[idx].saved_variables[vidx].variable_name);
}

LUA_FUNC(getVariableType, l_Cmission, "Variable number (Zero-based)", "Variable type (string)", "Gets the type of the variable (Number or string)")
{
	int idx = -1;
	int vidx = -1;
	if(!lua_get_args(L, "oi", &idx, &vidx))
		return LUA_RETURN_NIL;

	if(idx < 0 || idx > Campaign.num_missions)
		return LUA_RETURN_NIL;

	if(vidx < 0 || vidx > Campaign.missions[idx].num_saved_variables)
		return LUA_RETURN_NIL;

	char buf[NAME_LENGTH];

	if(Campaign.missions[idx].saved_variables[vidx].type & SEXP_VARIABLE_NUMBER)
		strcpy(buf, "Number");
	if(Campaign.missions[idx].saved_variables[vidx].type & SEXP_VARIABLE_STRING)
		strcpy(buf, "String");

	return lua_set_args(L, "i", Campaign.missions[idx].saved_variables[vidx].variable_name);
}

LUA_FUNC(getVariableValue, l_Cmission, "Variable number (Zero-based)", "Variable value (number or string)", "Gets the value of a variable")
{
	int idx = -1;
	int vidx = -1;
	if(!lua_get_args(L, "oi", &idx, &vidx))
		return LUA_RETURN_NIL;

	if(idx < 0 || idx > Campaign.num_missions)
		return LUA_RETURN_NIL;

	if(vidx < 0 || vidx > Campaign.missions[idx].num_saved_variables)
		return LUA_RETURN_NIL;

	if(Campaign.missions[idx].saved_variables[vidx].type & SEXP_VARIABLE_NUMBER)
		return lua_set_args(L, "i", atoi(Campaign.missions[idx].saved_variables[vidx].text));
	else if(Campaign.missions[idx].saved_variables[vidx].type & SEXP_VARIABLE_STRING)
		return lua_set_args(L, "s", atoi(Campaign.missions[idx].saved_variables[vidx].text));
	
	Warning(LOCATION, "LUA::getVariableName - Unknown variable type (%d) for variable (%s)", Campaign.missions[idx].saved_variables[vidx].type, Campaign.missions[idx].saved_variables[vidx].variable_name);
	return LUA_RETURN_FALSE;
}

//**********CLASS: Species
lua_obj<int> l_Species("species", "Species handle");
extern int Species_initted;

LUA_VAR(Name, l_Species, "String", "Species name")
{
	if(!Species_initted)
		return LUA_RETURN_NIL;

	char *s = NULL;
	int idx;
	if(!lua_get_args(L, "o|s", l_Species.Get(&idx), &s))
		return LUA_RETURN_NIL;

	if(idx < 0 || idx > Num_species)
		return LUA_RETURN_NIL;

	if(LUA_SETTING_VAR && s != NULL) {
		strncpy(Species_info[idx].species_name, s, sizeof(Species_info[idx].species_name)-1);
	}

	return lua_set_args(L, "s", Species_info[idx].species_name);
}

//**********CLASS: Shiptype
lua_obj<int> l_Shiptype("shiptype", "Ship type handle");
extern int Species_initted;

LUA_VAR(Name, l_Shiptype, "String", "Ship type name")
{
	if(!Species_initted)
		return LUA_RETURN_NIL;

	char *s = NULL;
	int idx;
	if(!lua_get_args(L, "o|s", l_Shiptype.Get(&idx), &s))
		return LUA_RETURN_NIL;

	if(idx < 0 || idx > (int)Ship_types.size())
		return LUA_RETURN_NIL;

	if(LUA_SETTING_VAR && s != NULL) {
		strncpy(Ship_types[idx].name, s, sizeof(Ship_types[idx].name)-1);
	}

	return lua_set_args(L, "s", Ship_types[idx].name);
}

//**********CLASS: Shipclass
lua_obj<int> l_Shipclass("shipclass", "Ship class handle");
extern int ships_inited;

LUA_VAR(Name, l_Shipclass, "String", "Ship class name")
{
	int idx;
	char *s = NULL;
	if(!lua_get_args(L, "o|s", l_Shipclass.Get(&idx), &s))
		return LUA_RETURN_NIL;

	if(idx < 0 || idx > Num_ship_classes)
		return LUA_RETURN_NIL;

	if(LUA_SETTING_VAR && s != NULL) {
		strncpy(Ship_info[idx].name, s, sizeof(Ship_info[idx].name)-1);
	}

	return lua_set_args(L, "s", Ship_info[idx].name);
}

LUA_VAR(ShortName, l_Shipclass, "String", "Ship class short name")
{
	int idx;
	char *s = NULL;
	if(!lua_get_args(L, "o|s", l_Shipclass.Get(&idx), &s))
		return LUA_RETURN_NIL;

	if(idx < 0 || idx > Num_ship_classes)
		return LUA_RETURN_NIL;

	if(LUA_SETTING_VAR && s != NULL) {
		strncpy(Ship_info[idx].short_name, s, sizeof(Ship_info[idx].short_name)-1);
	}

	return lua_set_args(L, "s", Ship_info[idx].short_name);
}

LUA_VAR(TypeString, l_Shipclass, "String", "Ship class type string")
{
	int idx;
	char *s = NULL;
	if(!lua_get_args(L, "o|s", l_Shipclass.Get(&idx), &s))
		return LUA_RETURN_NIL;

	if(idx < 0 || idx > Num_ship_classes)
		return LUA_RETURN_NIL;

	ship_info *sip = &Ship_info[idx];

	if(LUA_SETTING_VAR) {
		vm_free(sip->type_str);
		if(s != NULL) {
			sip->type_str = (char*)vm_malloc(strlen(s)+1);
			strcpy(sip->type_str, s);
		} else {
			sip->type_str = NULL;
		}
	}

	if(sip->type_str != NULL)
		return lua_set_args(L, "s", sip->type_str);
	else
		return lua_set_args(L, "s", "");
}

LUA_VAR(ManeuverabilityString, l_Shipclass, "String", "Ship class maneuverability string")
{
	int idx;
	char *s = NULL;
	if(!lua_get_args(L, "o|s", l_Shipclass.Get(&idx), &s))
		return LUA_RETURN_NIL;

	if(idx < 0 || idx > Num_ship_classes)
		return LUA_RETURN_NIL;

	ship_info *sip = &Ship_info[idx];

	if(LUA_SETTING_VAR) {
		vm_free(sip->maneuverability_str);
		if(s != NULL) {
			sip->maneuverability_str = (char*)vm_malloc(strlen(s)+1);
			strcpy(sip->maneuverability_str, s);
		} else {
			sip->maneuverability_str = NULL;
		}
	}

	if(sip->maneuverability_str != NULL)
		return lua_set_args(L, "s", sip->maneuverability_str);
	else
		return lua_set_args(L, "s", "");
}

LUA_VAR(ArmorString, l_Shipclass, "String", "Ship class armor string")
{
	int idx;
	char *s = NULL;
	if(!lua_get_args(L, "o|s", l_Shipclass.Get(&idx), &s))
		return LUA_RETURN_NIL;

	if(idx < 0 || idx > Num_ship_classes)
		return LUA_RETURN_NIL;

	ship_info *sip = &Ship_info[idx];

	if(LUA_SETTING_VAR) {
		vm_free(sip->armor_str);
		if(s != NULL) {
			sip->armor_str = (char*)vm_malloc(strlen(s)+1);
			strcpy(sip->armor_str, s);
		} else {
			sip->armor_str = NULL;
		}
	}

	if(sip->armor_str != NULL)
		return lua_set_args(L, "s", sip->armor_str);
	else
		return lua_set_args(L, "s", "");
}

LUA_VAR(ManufacturerString, l_Shipclass, "String", "Ship class manufacturer")
{
	int idx;
	char *s = NULL;
	if(!lua_get_args(L, "o|s", l_Shipclass.Get(&idx), &s))
		return LUA_RETURN_NIL;

	if(idx < 0 || idx > Num_ship_classes)
		return LUA_RETURN_NIL;

	ship_info *sip = &Ship_info[idx];

	if(LUA_SETTING_VAR) {
		vm_free(sip->manufacturer_str);
		if(s != NULL) {
			sip->manufacturer_str = (char*)vm_malloc(strlen(s)+1);
			strcpy(sip->manufacturer_str, s);
		} else {
			sip->manufacturer_str = NULL;
		}
	}

	if(sip->manufacturer_str != NULL)
		return lua_set_args(L, "s", sip->manufacturer_str);
	else
		return lua_set_args(L, "s", "");
}


LUA_VAR(Description, l_Shipclass, "String", "Ship class description")
{
	int idx;
	char *s = NULL;
	if(!lua_get_args(L, "o|s", l_Shipclass.Get(&idx), &s))
		return LUA_RETURN_NIL;

	if(idx < 0 || idx > Num_ship_classes)
		return LUA_RETURN_NIL;

	ship_info *sip = &Ship_info[idx];

	if(LUA_SETTING_VAR) {
		vm_free(sip->desc);
		if(s != NULL) {
			sip->desc = (char*)vm_malloc(strlen(s)+1);
			strcpy(sip->desc, s);
		} else {
			sip->desc = NULL;
		}
	}

	if(sip->desc != NULL)
		return lua_set_args(L, "s", sip->desc);
	else
		return lua_set_args(L, "s", "");
}

LUA_VAR(TechDescription, l_Shipclass, "String", "Ship class tech description")
{
	int idx;
	char *s = NULL;
	if(!lua_get_args(L, "o|s", l_Shipclass.Get(&idx), &s))
		return LUA_RETURN_NIL;

	if(idx < 0 || idx > Num_ship_classes)
		return LUA_RETURN_NIL;

	ship_info *sip = &Ship_info[idx];

	if(LUA_SETTING_VAR) {
		vm_free(sip->tech_desc);
		if(s != NULL) {
			sip->tech_desc = (char*)vm_malloc(strlen(s)+1);
			strcpy(sip->tech_desc, s);
		} else {
			sip->tech_desc = NULL;
		}
	}

	if(sip->tech_desc != NULL)
		return lua_set_args(L, "s", sip->tech_desc);
	else
		return lua_set_args(L, "s", "");
}

LUA_VAR(Hitpoints, l_Shipclass, "Number", "Ship class hitpoints")
{
	int idx;
	float f = -1.0f;
	if(!lua_get_args(L, "o|f", l_Shipclass.Get(&idx), &f))
		return LUA_RETURN_NIL;

	if(idx < 0 || idx > Num_ship_classes)
		return LUA_RETURN_NIL;

	if(LUA_SETTING_VAR && f >= 0.0f) {
		Ship_info[idx].max_hull_strength = f;
	}

	return lua_set_args(L, "f", Ship_info[idx].max_hull_strength);
}

LUA_VAR(Species, l_Shipclass, "Species", "Ship class species")
{
	int idx;
	int sidx;
	if(!lua_get_args(L, "o|o", l_Shipclass.Get(&idx), l_Species.Get(&sidx)))
		return LUA_RETURN_NIL;

	if(idx < 0 || idx > Num_ship_classes)
		return LUA_RETURN_NIL;

	if(LUA_SETTING_VAR && sidx > -1 && sidx < Num_species) {
		Ship_info[idx].species = sidx;
	}

	return lua_set_args(L, "o", l_Species.Set(Ship_info[idx].species));
}

LUA_VAR(Type, l_Shipclass, "shiptype", "Ship class type")
{
	int idx;
	int sidx;
	if(!lua_get_args(L, "o|o", l_Shipclass.Get(&idx), l_Shiptype.Get(&sidx)))
		return LUA_RETURN_NIL;

	if(idx < 0 || idx > Num_ship_classes)
		return LUA_RETURN_NIL;

	if(LUA_SETTING_VAR && sidx > -1 && sidx < (int)Ship_types.size()) {
		Ship_info[idx].class_type = sidx;
	}

	return lua_set_args(L, "o", l_Shiptype.Set(Ship_info[idx].class_type));
}

LUA_FUNC(isInTechroom, l_Shipclass, NULL, "Whether ship has been revealed in the techroom", "Gets whether or not the ship class is available in the techroom")
{
	int idx;
	if(!lua_get_args(L, "o", l_Shipclass.Get(&idx)))
		return LUA_RETURN_NIL;

	if(idx < 0 || idx > Num_ship_classes)
		return LUA_RETURN_NIL;

	bool b = false;
	if(Player != NULL && (Player->flags & PLAYER_FLAGS_IS_MULTI) && (Ship_info[idx].flags & SIF_IN_TECH_DATABASE_M)) {
		b = true;
	} else if(Ship_info[idx].flags & SIF_IN_TECH_DATABASE) {
		b = true;
	}

	return lua_set_args(L, "b", b);
}


LUA_FUNC(renderTechModel, l_Shipclass, "X1, Y1, X2, Y2, [Resize], [Rotation %], [Pitch %], [Bank %], [Zoom multiplier]", "Whether ship was rendered", "Draws ship model as if in techroom")
{
	int x1,y1,x2,y2;
	angles rot_angles = {0.0f, 0.0f, 40.0f};
	bool r;
	int idx;
	float zoom = 1.3f;
	if(!lua_get_args(L, "oiiii|bffff", l_Shipclass.Get(&idx), &x1, &y1, &x2, &y2, &r, &rot_angles.h, &rot_angles.p, &rot_angles.b, &zoom))
		return LUA_RETURN_NIL;

	if(idx < 0 || idx > Num_ship_classes)
		return lua_set_args(L, "b", false);

	if(x2 < x1 || y2 < y1)
		return lua_set_args(L, "b", false);

	if(rot_angles.p < 0.0f)
		rot_angles.p = 0.0f;
	if(rot_angles.p > 100.0f)
		rot_angles.p = 100.0f;
	if(rot_angles.b < 0.0f)
		rot_angles.b = 0.0f;
	if(rot_angles.b > 100.0f)
		rot_angles.b = 100.0f;
	if(rot_angles.h < 0.0f)
		rot_angles.h = 0.0f;
	if(rot_angles.h > 100.0f)
		rot_angles.h = 100.0f;

	ship_info *sip = &Ship_info[idx];

	//Make sure model is loaded
	sip->modelnum = model_load(sip->pof_file, sip->n_subsystems, &sip->subsystems[0], 0);

	if(sip->modelnum < 0)
		return lua_set_args(L, "b", false);

	//Handle angles
	matrix orient = vmd_identity_matrix;
	angles view_angles = {-0.6f, 0.0f, 0.0f};
	vm_angles_2_matrix(&orient, &view_angles);

	rot_angles.p = (rot_angles.p*0.01f) * PI2;
	rot_angles.b = (rot_angles.b*0.01f) * PI2;
	rot_angles.h = (rot_angles.h*0.01f) * PI2;
	vm_rotate_matrix_by_angles(&orient, &rot_angles);

	//Clip
	gr_set_clip(x1,y1,x2-x1,y2-y1,r);

	//Handle 3D init stuff
	g3_start_frame(1);
	g3_set_view_matrix(&sip->closeup_pos, &vmd_identity_matrix, sip->closeup_zoom * zoom);
	if (!Cmdline_nohtl) gr_set_proj_matrix( (4.0f/9.0f) * 3.14159f * View_zoom, gr_screen.aspect*(float)gr_screen.clip_width/(float)gr_screen.clip_height, Min_draw_distance, Max_draw_distance);
	if (!Cmdline_nohtl)	gr_set_view_matrix(&Eye_position, &Eye_matrix);

	//Handle light
	light_reset();
	vec3d light_dir = vmd_zero_vector;
	light_dir.xyz.y = 1.0f;	
	light_add_directional(&light_dir, 0.65f, 1.0f, 1.0f, 1.0f);
	light_rotate_all();

	//Draw the ship!!
	model_clear_instance(sip->modelnum);
	model_set_detail_level(0);
	model_render(sip->modelnum, &orient, &vmd_zero_vector, MR_LOCK_DETAIL | MR_AUTOCENTER | MR_NO_FOGGING);

	//OK we're done
	if (!Cmdline_nohtl) 
	{
		gr_end_view_matrix();
		gr_end_proj_matrix();
	}

	//Bye!!
	g3_end_frame();
	gr_reset_clip();

	return lua_set_args(L, "b", true);
}

//**********CLASS: Shields
lua_obj<object_h> l_Shields("shields", "Shields handle");

LUA_INDEXER(l_Shields, "Shield quadrant")
{
	object_h *objh;
	char *qd = NULL;
	float nval = -1.0f;
	if(!lua_get_args(L, "os|f", l_Shields.GetPtr(&objh), &qd, &nval))
		return 0;

	if(!objh->IsValid())
		return LUA_RETURN_NIL;

	object *objp = objh->objp;

	//Which quadrant?
	int qdx;
	int qdi;
	if(qd == NULL || !stricmp(qd, "None"))
		qdx = -1;
	else if((qdi = atoi(qd)) > 0 && qdi < 5)
		qdx = qdi-1;	//LUA->FS2
	else if(!stricmp(qd, "Top"))
		qdx = 0;
	else if(!stricmp(qd, "Left"))
		qdx = 1;
	else if(!stricmp(qd, "Right"))
		qdx = 2;
	else if(!stricmp(qd, "Bottom"))
		qdx = 3;
	else
		return LUA_RETURN_NIL;

	//Set/get all quadrants
	if(qdx == -1) {
		if(LUA_SETTING_VAR && nval >= 0.0f)
			set_shield_strength(objp, nval);

		return lua_set_args(L, "f", get_shield_strength(objp));
	}

	//Set one quadrant?
	if(LUA_SETTING_VAR && nval >= 0.0f)
		objp->shield_quadrant[qdx] = nval;

	//Get one quadrant
	return lua_set_args(L, "f", objp->shield_quadrant[qdx]);
}

//**********CLASS: Object
lua_obj<object_h> l_Object("object", "Object");
//Helper function
//Returns 1 if object sig stored in idx exists, and stores Objects[] index in idx
//Returns 0 if object sig does not exist, and does not change idx

LUA_VAR(Position, l_Object, "World vector", "Object world position")
{
	object_h *objh;
	vec3d *v3=NULL;
	if(!lua_get_args(L, "o|o", l_Object.GetPtr(&objh), l_Vector.GetPtr(&v3)))
		return LUA_RETURN_NIL;

	if(!objh->IsValid() || v3==NULL)
		return LUA_RETURN_NIL;

	if(LUA_SETTING_VAR) {
		objh->objp->pos = *v3;
	}

	return lua_set_args(L, "o", l_Vector.Set(objh->objp->pos));
}

LUA_VAR(Velocity, l_Object, "World vector", "Object world velocity")
{
	object_h *objh;
	vec3d *v3=NULL;
	if(!lua_get_args(L, "o|o", l_Object.GetPtr(&objh), l_Vector.GetPtr(&v3)))
		return LUA_RETURN_NIL;

	if(!objh->IsValid() || v3==NULL)
		return LUA_RETURN_NIL;

	if(LUA_SETTING_VAR) {
		objh->objp->phys_info.vel = *v3;
	}

	return lua_set_args(L, "o", l_Vector.Set(objh->objp->phys_info.vel));
}

LUA_VAR(MaxVelocity, l_Object, "Local vector", "Object max local velocity")
{
	object_h *objh;
	vec3d *v3=NULL;
	if(!lua_get_args(L, "o|o", l_Object.GetPtr(&objh), l_Vector.GetPtr(&v3)))
		return LUA_RETURN_NIL;

	if(!objh->IsValid() || v3==NULL)
		return LUA_RETURN_NIL;

	if(LUA_SETTING_VAR) {
		objh->objp->phys_info.max_vel = *v3;
	}

	return lua_set_args(L, "o", l_Vector.Set(objh->objp->phys_info.max_vel));
}


LUA_VAR(HitpointsLeft, l_Object, "Number", "Hitpoints an object has left")
{
	object_h *objh;
	float f = -1.0f;
	if(!lua_get_args(L, "o|f", l_Object.GetPtr(&objh), &f))
		return LUA_RETURN_NIL;

	if(!objh->IsValid() || f < 0.0f)
		return LUA_RETURN_NIL;

	//Set hull strength.
	if(LUA_SETTING_VAR && f >= 0.0f) {
		objh->objp->hull_strength = f;
	}

	return lua_set_args(L, "f", objh->objp->hull_strength);
}

LUA_VAR(Shields, l_Object, "shields", "Shields")
{
	object_h *objh;
	object_h *sobjh;
	if(!lua_get_args(L, "o|o", l_Object.GetPtr(&objh), l_Shields.GetPtr(&sobjh)))
		return LUA_RETURN_NIL;

	if(!objh->IsValid())
		return LUA_RETURN_NIL;

	//WMC - copy shields
	if(LUA_SETTING_VAR && sobjh->IsValid())
	{
		for(int i = 0; i < 4; i++)
		{
			objh->objp->shield_quadrant[i] = sobjh->objp->shield_quadrant[i];
		}
	}

	return lua_set_args(L, "o", l_Shields.Set(object_h(objh->objp)));
}

LUA_FUNC(getBreed, l_Object, NULL, "Object type name", "Gets object type")
{
	object_h *objh;
	if(!lua_get_args(L, "o", l_Object.GetPtr(&objh)))
		return LUA_RETURN_NIL;

	if(!objh->IsValid())
		return LUA_RETURN_NIL;

	return lua_set_args(L, "s", Object_type_names[objh->objp->type]);
}

LUA_FUNC(fetchShieldStrength, l_Object, "[Shield Quadrant], [New value]", "[New] shield strength",
	"Returns total shield strength if no quadrant is specified, or individual quadrant strength if one is specified."
	"Valid quadrants are \"Front\", \"Back\", \"Left\", and \"Right\". Specifying a new value will set the specified quadrant to that amount. "
	"\"None\" may be used for the new value to be divided equally between all quadrants")
{
	object_h *objh;
	char *qd = NULL;
	float nval = -1.0f;
	if(!lua_get_args(L, "o|sf", l_Object.GetPtr(&objh), &qd, &nval))
		return 0;

	if(!objh->IsValid())
		return LUA_RETURN_NIL;

	object *objp = objh->objp;

	//Which quadrant?
	int qdx=-1;
	if(qd == NULL || !stricmp(qd, "None"))
		qdx = -1;
	else if(!stricmp(qd, "Top"))
		qdx = 0;
	else if(!stricmp(qd, "Left"))
		qdx = 1;
	else if(!stricmp(qd, "Right"))
		qdx = 2;
	else if(!stricmp(qd, "Bottom"))
		qdx = 3;
	else
		return LUA_RETURN_NIL;

	//Set/get all quadrants
	if(qdx == -1) {
		if(nval >= 0.0f)
			set_shield_strength(objp, nval);

		return lua_set_args(L, "f", get_shield_strength(objp));
	}

	//Set one quadrant?
	if(nval >= 0.0f)
		objp->shield_quadrant[qdx] = nval;

	//Get one quadrant
	return lua_set_args(L, "f", objp->shield_quadrant[qdx]);
}

//**********CLASS: Mounted Weapons
struct ship_weapon_h : public object_h
{
	ship_weapon *sw;	//Pointer to subsystem, or NULL for the hull

	bool IsValid(){return objp->signature == sig;}
	ship_weapon_h(object *objp, ship_weapon *wpn) : object_h(objp)
	{
		sw = wpn;
	}
};

lua_obj<ship_weapon_h> l_MountedWeapons("mountedweapons", "Mounted weapons on a ship or subsystem");

LUA_FUNC(getNumWeapons, l_MountedWeapons, "[Type]", "Number of weapons, or false if invalid type specified",
		 "Gets total number of weapons mounted. For a specific type, use \"Primary\", \"Secondary\", or \"Tertiary\".")
{
	ship_weapon_h *mw;
	char *t = NULL;
	if(!lua_get_args(L, "o|s", l_MountedWeapons.GetPtr(&mw), &t))
		return LUA_RETURN_NIL;

	if(!mw->IsValid())
		return LUA_RETURN_NIL;

	ship_weapon *sw = mw->sw;

	//Now do stuff
	//All weapons
	if(t == NULL) {
		return lua_set_args(L, "i", sw->num_primary_banks + sw->num_secondary_banks + sw->num_tertiary_banks);
	}

	//Just one type
	if(!stricmp(t, "Primary"))
		return lua_set_args(L, "i", sw->num_primary_banks);
	else if(!stricmp(t, "Secondary"))
		return lua_set_args(L, "i", sw->num_secondary_banks);
	else if(!stricmp(t, "Tertiary"))
		return lua_set_args(L, "i", sw->num_tertiary_banks);

	return LUA_RETURN_FALSE;
}

LUA_FUNC(getBankName, l_MountedWeapons, "Type, Index", "Weapon name, or false if no weapon is mounted at that index, or an invalid type is specified",
		 "Gets weapon name for specified mount index of type. Use \"Primary\" or \"Secondary\" for type.")
{
	ship_weapon_h *mw;
	char *t = NULL;
	int i = 0;
	if(!lua_get_args(L, "osi", l_MountedWeapons.GetPtr(&mw), &t, &i))
		return LUA_RETURN_NIL;

	if(!mw->IsValid())
		return LUA_RETURN_NIL;

	ship_weapon *sw = mw->sw;

	if(i < 1)
		return LUA_RETURN_FALSE;

	//Lua->FS2
	i--;

	if(!stricmp(t, "Primary") && i < sw->num_primary_banks && sw->primary_bank_weapons[i] > -1)
		return lua_set_args(L, "s", Weapon_info[sw->primary_bank_weapons[i]].name);
	else if(!stricmp(t, "Secondary") && i < sw->num_secondary_banks && sw->secondary_bank_weapons[i] > -1)
		return lua_set_args(L, "s", Weapon_info[sw->secondary_bank_weapons[i]].name);

	//Invalid type or weapon missing
	return LUA_RETURN_FALSE;
}

LUA_FUNC(fetchCurrentWeapon, l_MountedWeapons, "Type, [New index]", "Mount index",
		 "Gets currently armed weapon of type. Use \"Primary\", \"Secondary\", or \"Tertiary\" for type.")
{
	ship_weapon_h *mw;
	char *t = NULL;
	int i = 0;
	if(!lua_get_args(L, "os|i", l_MountedWeapons.GetPtr(&mw), &t, &i))
		return LUA_RETURN_NIL;

	if(!mw->IsValid())
		return LUA_RETURN_NIL;

	ship_weapon *sw = mw->sw;

	//Lua->FS2
	i--;

	if(!stricmp(t, "Primary")) {
		if( i > -1 && i < sw->num_primary_banks && sw->primary_bank_weapons[i] > -1)
			sw->current_primary_bank = i;
		return lua_set_args(L, "i", sw->current_primary_bank);
	}
	if(!stricmp(t, "Secondary")) {
		if( i > -1 && i < sw->num_secondary_banks && sw->secondary_bank_weapons[i] > -1)
			sw->current_secondary_bank = i;
		return lua_set_args(L, "i", sw->current_secondary_bank);
	}
	if(!stricmp(t, "Tertiary")) {
		if( i > -1 && i < sw->num_tertiary_banks)
			sw->current_tertiary_bank = i;
		return lua_set_args(L, "i", sw->current_tertiary_bank);
	}

	return LUA_RETURN_FALSE;
}

LUA_FUNC(fetchBankAmmo, l_MountedWeapons, "Type, Index, [New ammo amount]", "[New] ammo amount",
		 "Gets weapon ammo, or sets to a new amount if specified.. Use \"Primary\", \"Secondary\", or \"Tertiary\" for type.")
{
	ship_weapon_h *mw;
	char *t = NULL;
	int i = 0;
	int a = -1;
	if(!lua_get_args(L, "osi|i", l_MountedWeapons.GetPtr(&mw), &t, &i, &a))
		return LUA_RETURN_NIL;

	if(!mw->IsValid())
		return LUA_RETURN_NIL;

	ship_weapon *sw = mw->sw;
	if(i < 1)
		return LUA_RETURN_FALSE;

	//Lua->FS2
	i--;

	//Get/set
	if(!stricmp(t, "Primary") && i < sw->num_primary_banks && sw->primary_bank_weapons[i] > -1) {
		if(a > -1)
			sw->primary_bank_ammo[i] = a;
		return lua_set_args(L, "i", sw->primary_bank_ammo[i]);
	} else if(!stricmp(t, "Secondary") && i < sw->num_secondary_banks && sw->secondary_bank_weapons[i] > -1) {
		if(a > -1)
			sw->secondary_bank_ammo[i] = a;
		return lua_set_args(L, "i", sw->secondary_bank_ammo[i]);
	} else if(!stricmp(t, "Tertiary") && i == 0) {
		if(a > -1)
			sw->tertiary_bank_ammo = a;
		return lua_set_args(L, "i", sw->tertiary_bank_ammo);
	}

	//Invalid type or weapon missing
	return LUA_RETURN_FALSE;
}

LUA_FUNC(fetchBankCapacity, l_MountedWeapons, "Type, Index, [New ammo capacity]", "[New] ammo capacity",
		 "Gets weapon capacity, or sets to a new amount if specified.. Use \"Primary\", \"Secondary\", or \"Tertiary\" for type.")
{
	ship_weapon_h *mw;
	char *t = NULL;
	int i = 0;
	int a = -1;
	if(!lua_get_args(L, "osi|i", l_MountedWeapons.GetPtr(&mw), &t, &i, &a))
		return LUA_RETURN_NIL;

	if(!mw->IsValid())
		return LUA_RETURN_NIL;

	ship_weapon *sw = mw->sw;

	if(i < 1)
		return LUA_RETURN_FALSE;

	//Lua->FS2
	i--;

	//Get/set
	if(!stricmp(t, "Primary") && i < sw->num_primary_banks && sw->primary_bank_weapons[i] > -1) {
		if(a > -1)
			sw->primary_bank_start_ammo[i] = a;
		return lua_set_args(L, "i", sw->primary_bank_start_ammo[i]);
	} else if(!stricmp(t, "Secondary") && i < sw->num_secondary_banks && sw->secondary_bank_weapons[i] > -1) {
		if(a > -1)
			sw->secondary_bank_start_ammo[i] = a;
		return lua_set_args(L, "i", sw->secondary_bank_start_ammo[i]);
	} else if(!stricmp(t, "Tertiary") && i == 0) {
		if(a > -1)
			sw->tertiary_bank_start_ammo = a;
		return lua_set_args(L, "i", sw->tertiary_bank_start_ammo);
	}

	//Invalid type or weapon missing
	return LUA_RETURN_FALSE;
}
//**********CLASS: Subsystem
struct ship_subsys_h : public object_h
{
	ship_subsys *ss;	//Pointer to subsystem, or NULL for the hull

	bool IsValid(){return objp->signature == sig;}
	ship_subsys_h(object *objp, ship_subsys *sub) : object_h(objp) {
		ss = sub;
	}
};
lua_obj<ship_subsys_h> l_Subsystem("subsystem", "Ship subsystem object");

LUA_VAR(Target, l_Subsystem, "Object", "Object targetted by this subsystem")
{
	ship_subsys_h *sso;
	object_h *objh;
	if(!lua_get_args(L, "o|o", l_Subsystem.GetPtr(&sso), l_Object.GetPtr(&objh)))
		return LUA_RETURN_NIL;

	if(!sso->IsValid())
		return LUA_RETURN_NIL;

	ship_subsys *ss = sso->ss;

	if(LUA_SETTING_VAR && objh->IsValid())
	{
		ss->turret_enemy_objnum = OBJ_INDEX(objh->objp);
		ss->turret_enemy_sig = objh->sig;
		ss->targeted_subsys = NULL;
	}

	return lua_set_args(L, "f", ss->current_hits);
}

LUA_VAR(HitpointsLeft, l_Subsystem, "Number", "Subsystem hitpoints left")
{
	ship_subsys_h *sso;
	float f = -1.0f;
	if(!lua_get_args(L, "o|f", l_Subsystem.GetPtr(&sso), &f))
		return LUA_RETURN_NIL;

	if(!sso->IsValid())
		return LUA_RETURN_NIL;

	if(LUA_SETTING_VAR && f >= 0.0f)
		sso->ss->current_hits = f;

	return lua_set_args(L, "f", sso->ss->current_hits);
}

LUA_VAR(HitpointsMax, l_Subsystem, "Number", "Subsystem hitpoints max")
{
	ship_subsys_h *sso;
	float f = -1.0f;
	if(!lua_get_args(L, "o|f", l_Subsystem.GetPtr(&sso), &f))
		return LUA_RETURN_NIL;

	if(!sso->IsValid())
		return LUA_RETURN_NIL;

	if(LUA_SETTING_VAR && f >= 0.0f)
		sso->ss->max_hits = f;

	return lua_set_args(L, "f", sso->ss->max_hits);
}

LUA_VAR(AWACSIntensity, l_Subsystem, "Number", "Subsystem AWACS intensity")
{
	ship_subsys_h *sso;
	float f = -1.0f;
	if(!lua_get_args(L, "o|f", l_Subsystem.GetPtr(&sso), &f))
		return LUA_RETURN_NIL;

	if(!sso->IsValid())
		return LUA_RETURN_NIL;

	if(LUA_SETTING_VAR && f >= 0.0f)
		sso->ss->awacs_intensity = f;

	return lua_set_args(L, "f", sso->ss->awacs_intensity);
}

LUA_VAR(AWACSRadius, l_Subsystem, "Number", "Subsystem AWACS radius")
{
	ship_subsys_h *sso;
	float f = -1.0f;
	if(!lua_get_args(L, "o|f", l_Subsystem.GetPtr(&sso), &f))
		return LUA_RETURN_NIL;

	if(!sso->IsValid())
		return LUA_RETURN_NIL;

	if(LUA_SETTING_VAR && f >= 0.0f)
		sso->ss->awacs_radius = f;

	return lua_set_args(L, "f", sso->ss->awacs_radius);
}

//**********CLASS: Ship
lua_obj<object_h> l_Ship("ship", "Ship object", &l_Object);

LUA_FUNC(fetchName, l_Ship, "[New name]", "[New] ship name (string)", "Gets ship name")
{
	object_h *objh;
	char *s = NULL;
	if(!lua_get_args(L, "o|f", l_Ship.GetPtr(&objh), &s))
		return LUA_RETURN_NIL;

	if(!objh->IsValid())
		return LUA_RETURN_NIL;

	ship *shipp = &Ships[objh->objp->instance];

	if(s == NULL) {
		strncpy(shipp->ship_name, s, sizeof(shipp->ship_name)-1);
	}

	return lua_set_args(L, "s", shipp->ship_name);
}

LUA_FUNC(getClass, l_Ship, NULL, "Ship class handle (shipclass)", "Gets ship class handle")
{
	object_h *objh;
	if(!lua_get_args(L, "o|f", l_Ship.GetPtr(&objh)))
		return LUA_RETURN_NIL;

	if(!objh->IsValid())
		return LUA_RETURN_NIL;

	return lua_set_args(L, "o", l_Shipclass.Set(Ships[objh->objp->instance].ship_info_index));
}

LUA_FUNC(getMountedWeapons, l_Ship, "[Subsystem name]", "mountedweapons object, or false if invalid subsystem specified", "Gets weapons mounted on a ship or subsystem")
{
	object_h *objh;
	char *s=NULL;
	if(!lua_get_args(L, "o|s", l_Ship.GetPtr(&objh), &s))
		return 0;

	if(!objh->IsValid())
		return LUA_RETURN_NIL;

	ship_weapon_h mw(objh->objp, NULL);

	if(s == NULL)
	{
		mw.sw = NULL;
		return lua_set_args(L, "o", l_MountedWeapons.Set(mw));
	}

	ship_subsys *ss = ship_get_subsys(&Ships[objh->objp->instance], s);
	if(ss == NULL)
		return LUA_RETURN_FALSE;

	mw.sw = &ss->weapons;

	return lua_set_args(L, "o", l_MountedWeapons.Set(mw));
}

LUA_FUNC(fetchAfterburnerFuel, l_Ship, "[Fuel amount]", "[New] fuel amount", "Returns ship fuel amount, or sets it if amount is specified")
{
	object_h *objh;
	float fuel = -1.0f;
	if(!lua_get_args(L, "o|f", l_Ship.GetPtr(&objh), &fuel))
		return LUA_RETURN_NIL;

	if(!objh->IsValid())
		return LUA_RETURN_NIL;

	ship *shipp = &Ships[objh->objp->instance];

	if(fuel >= 0.0f)
		shipp->afterburner_fuel = fuel;

	return lua_set_args(L, "f", shipp->afterburner_fuel);
}

LUA_FUNC(warpIn, l_Ship, NULL, "True", "Warps ship in")
{
	object_h *objh;
	if(!lua_get_args(L, "o", l_Ship.GetPtr(&objh)))
		return LUA_RETURN_NIL;

	if(!objh->IsValid())
		return LUA_RETURN_NIL;

	shipfx_warpin_start(objh->objp);

	return LUA_RETURN_TRUE;
}

LUA_FUNC(warpOut, l_Ship, NULL, "True", "Warps ship out")
{
	object_h *objh;
	if(!lua_get_args(L, "o", l_Ship.GetPtr(&objh)))
		return LUA_RETURN_NIL;

	if(!objh->IsValid())
		return LUA_RETURN_NIL;

	shipfx_warpout_start(objh->objp);

	return LUA_RETURN_TRUE;
}

//**********OBJECT: Player
lua_obj<int> l_Player("player", "Player object");

int player_helper(lua_State *L, int *idx)
{
	if(!lua_get_args(L, "o", l_Player.Get(idx)))
		return 0;

	if(*idx < 0 || *idx > Player_num)
		return 0;

	return 1;
}

LUA_FUNC(getName, l_Player, NULL, "Player name (string)", "Gets current player name")
{
	int idx;
	player_helper(L, &idx);

	return lua_set_args(L, "s", Players[idx].callsign);
}

LUA_FUNC(getImage, l_Player, NULL, "Player image (string)", "Gets current player image")
{
	int idx;
	player_helper(L, &idx);

	return lua_set_args(L, "s", Players[idx].image_filename);
}

LUA_FUNC(getSquadronName, l_Player, NULL, "Squad name (string)", "Gets current player squad name")
{
	int idx;
	player_helper(L, &idx);

	return lua_set_args(L, "s", Players[idx].squad_name);
}
//WMC - This isn't working
/*
LUA_FUNC(getSquadronImage, l_Player, NULL, "Squad image (string)", "Gets current player squad image")
{
	int idx;
	player_helper(L, &idx);

	return lua_set_args(L, "s", Players[idx].squad_filename);
}*/

LUA_FUNC(getCampaignName, l_Player, NULL, "Campaign name (string)", "Gets current player campaign")
{
	int idx;
	player_helper(L, &idx);

	return lua_set_args(L, "s", Players[idx].current_campaign);
}

LUA_FUNC(getMainHall, l_Player, NULL, "Main hall number", "Gets player's main hall number")
{
	int idx;
	player_helper(L, &idx);

	return lua_set_args(L, "i", (int)Players[idx].main_hall);
}

//**********LIBRARY: Base
lua_lib l_Base("Base", "bs", "Base Freespace 2 functions");

LUA_FUNC(print, l_Base, "String", NULL, "Prints a string")
{
	Error(LOCATION, "LUA: %s", lua_tostring(L, -1));

	return LUA_RETURN_NIL;
}

LUA_FUNC(warning, l_Base, "String", NULL, "Displays a Freespace warning (debug build-only) message with the string provided")
{
	Warning(LOCATION, "LUA ERROR: %s", lua_tostring(L, -1));

	return LUA_RETURN_NIL;
}

LUA_FUNC(error, l_Base, "String", NULL, "Displays a Freespace error message with the string provided")
{
	Error(LOCATION, "LUA ERROR: %s", lua_tostring(L, -1));

	return LUA_RETURN_NIL;
}

LUA_FUNC(getFrametime, l_Base, "[Do not adjust for time compression (Boolean)]", "Frame time in seconds", "Gets how long this frame is calculated to take. Use it to for animations, physics, etc to make incremental changes.")
{
	bool b=false;
	lua_get_args(L, "|b", &b);

	return lua_set_args(L, "f", b ? flRealframetime : flFrametime);
}

LUA_FUNC(getState, l_Base, "[Depth (number)]", "State (string)", "Gets current Freespace state; if a depth is specified, the state at that depth is returned. (IE at the in-game options game, a depth of 1 would give you the game state, while the function defaults to 0, which would be the options screen.")
{
	int depth = 0;
	lua_get_args(L, "|i", &depth);

	return lua_set_args(L, "s", GS_state_text[gameseq_get_state(depth)]);
}

LUA_FUNC(setEvent, l_Base, "Event", "Whether a valid event name was given (boolean)", "Sets current game event. Note that you can crash Freespace 2 by setting a state at an improper time, so test extensively if you use it.")
{
	char *s;
	if(!lua_get_args(L, "s", &s))
		return LUA_RETURN_NIL;

	//WMC - I know it's not the best idea to check for state text
	//and then post using the event define, however, I figure
	//it's more modder-friendly than having them deal with 
	//two separate lists.
	for(int i = 0; i < Num_gs_event_text; i++)
	{
		if(!stricmp(s, GS_event_text[i])) {
			gameseq_post_event(i);
			return lua_set_args(L, "b", true);
		}
	}

	return lua_set_args(L, "b", false);
}

LUA_FUNC(getEventTypeName, l_Base, "Index of event type (number)", "Event name (string)", "Gets the name of a event type, given an index; this function may be used to list all event dealt with by setEvent()")
{
	int i;
	if(!lua_get_args(L, "i", &i))
		return LUA_RETURN_NIL;

	//Lua->FS2
	i--;

	if(i < 0 || i > Num_gs_state_text)
		return LUA_RETURN_NIL;

	return lua_set_args(L, "s", GS_event_text[i]);
}

LUA_FUNC(getNumEventTypes, l_Base, NULL, "Number of event types", "Gets the number of different event types currently implemented in FS2")
{
	return lua_set_args(L, "i", Num_gs_event_text);
}

LUA_FUNC(getCurrentPlayer, l_Base, NULL, "Current player", "Gets the current player")
{
	if(Player == NULL)
		return LUA_RETURN_NIL;

	int idx = Player - Players;
	return lua_set_args(L, "o", l_Player.Set(idx));
}

LUA_FUNC(getPlayerByIndex, l_Base, "Player index", "Player object", "Gets the named player")
{
	if(Player == NULL)
		return LUA_RETURN_NIL;

	int idx;
	if(!lua_get_args(L, "i", &idx))
		return LUA_RETURN_NIL;

	//Lua->FS2
	idx--;

	if(idx < 0 || idx > Player_num)
		return LUA_RETURN_NIL;

	return lua_set_args(L, "o", l_Player.Set(idx));
}

LUA_FUNC(getNumPlayers, l_Base, NULL, "Number of players", "Gets the number of currently loaded players")
{
	return lua_set_args(L, "i", Player_num);
}


//**********LIBRARY: Math
lua_lib l_Math("Math", "ma", "Math library");

LUA_FUNC(getRandomNumber, l_Math, "[Smallest number], [Largest number]", "Random number", "Returns a random number; default is 0 to 1")
{
	float min = 0.0f;
	float max = 1.0f;
	lua_get_args(L, "ff", &min, &max);

	if(max < min)
		return LUA_RETURN_NIL;

	return lua_set_args(L, "f", frand_range(min, max));
}

LUA_FUNC(newVector, l_Math, "[x], [y], [z]", "Vector object", "Creates a vector object")
{
	vec3d v3;
	lua_get_args(L, "|fff", &v3.xyz.x, &v3.xyz.y, &v3.xyz.z);

	return lua_set_args(L, "o", l_Vector.Set(v3));
}

//**********LIBRARY: Campaign
lua_lib l_Campaign("Campaign", "cn", "Campaign Library");

LUA_FUNC(getName, l_Campaign, NULL, "Campaign name", "Gets campaign name")
{
	return lua_set_args(L, "s", Campaign.name);
}

LUA_FUNC(getDescription, l_Campaign, NULL, "Campaign description or false if there is none", "Gets campaign description")
{
	if(Campaign.desc != NULL)
		return lua_set_args(L, "s", Campaign.desc);
	else
		return LUA_RETURN_FALSE;
}

LUA_FUNC(getNumMissions, l_Campaign, NULL, "Number of missions", "Gets the number of missions in the campaign")
{
	return lua_set_args(L, "i", Campaign.num_missions);
}

LUA_FUNC(getNumMissionsCompleted, l_Campaign, NULL, "Number of missions completed", "Gets the number of missions in the campaign that have been completed")
{
	return lua_set_args(L, "i", Campaign.num_missions_completed);
}

LUA_FUNC(getNextMissionName, l_Campaign, NULL, "Mission name, or false if there is no next mission", "Gets the name of the next mission in the campaign")
{
	if(Campaign.next_mission < 0)
		return LUA_RETURN_FALSE;

	return lua_set_args(L, "s", Campaign.missions[Campaign.next_mission].name);
}

LUA_FUNC(getNextMission, l_Campaign, NULL, "Cmission object, or false if there is no next mission", "Gets the next mission in the campaign")
{
	if(Campaign.next_mission < 0)
		return LUA_RETURN_FALSE;

	return lua_set_args(L, "o", l_Cmission.Set(Campaign.next_mission));
}

LUA_FUNC(getPrevMissionName, l_Campaign, NULL, "Mission name, or false if there is no next mission", "Gets the name of the next mission in the campaign")
{
	if(Campaign.prev_mission < 0)
		return LUA_RETURN_FALSE;

	return lua_set_args(L, "s", Campaign.missions[Campaign.prev_mission].name);
}

LUA_FUNC(getPrevMission, l_Campaign, NULL, "Cmission object, or false if there is no next mission", "Gets the previous mission in the campaign")
{
	if(Campaign.prev_mission < 0)
		return LUA_RETURN_FALSE;

	return lua_set_args(L, "o", l_Cmission.Set(Campaign.prev_mission));
}

LUA_FUNC(getMissionByName, l_Campaign, "Mission name", "Cmission object, or false if mission does not exist", "Gets the specified mission from the campaign by its name")
{
	char *s;

	if(!lua_get_args(L, "s", &s))
		return LUA_RETURN_NIL;

	for(int idx = 0; idx < Campaign.num_missions; idx++)
	{
		if(!stricmp(Campaign.missions[idx].name, s))
			return lua_set_args(L, "o", l_Cmission.Set(idx));
	}

	return LUA_RETURN_FALSE;
}


LUA_FUNC(getMissionByIndex, l_Campaign, "Mission number (Zero-based index)", "Cmission object", "Gets the specified mission by its index in the campaign")
{
	int idx;

	if(!lua_get_args(L, "i", &idx))
		return LUA_RETURN_NIL;

	//Lua->FS2
	idx--;

	if(idx < 0 || idx > Campaign.num_missions)
		return LUA_RETURN_NIL;

	return lua_set_args(L, "o", l_Cmission.Set(idx));
}

//**********LIBRARY: Mission
lua_lib l_Mission("Mission", "mn", "Mission library");

//WMC - These are in freespace.cpp
LUA_FUNC(loadMission, l_Mission, "Mission name", "True if mission was loaded, false otherwise", "Loads a mission")
{
	char *s;
	if(!lua_get_args(L, "s", &s))
		return LUA_RETURN_NIL;

	strncpy(Game_current_mission_filename, s, MAX_FILENAME_LEN-1);

	//NOW do the loading stuff
	game_stop_time();
	get_mission_info(s, &The_mission);
	game_level_init();

	if(mission_load(s) == -1)
		return LUA_RETURN_FALSE;

	game_post_level_init();

	Game_mode |= GM_IN_MISSION;

	return LUA_RETURN_TRUE;
}

LUA_FUNC(simulateFrame, l_Mission, NULL, NULL, "Simulates mission frame")
{
	game_update_missiontime();
	game_simulation_frame();

	return LUA_RETURN_TRUE;
}

LUA_FUNC(renderFrame, l_Mission, NULL, NULL, "Renders mission frame, but does not move anything")
{
	vec3d eye_pos;
	matrix eye_orient;
	game_render_frame_setup(&eye_pos, &eye_orient);
	game_render_frame( &eye_pos, &eye_orient );
	game_render_post_frame();

	return LUA_RETURN_TRUE;
}

LUA_FUNC(getShip, l_Mission, "Ship name", "Ship object", "Gets ship object")
{
	char *name;
	if(!lua_get_args(L, "s", &name))
		return 0;

	int idx = ship_name_lookup(name);
	
	if(idx < 0) {
		return LUA_RETURN_NIL;
	}

	return lua_set_args(L, "o", l_Ship.Set(object_h(&Objects[Ships[idx].objnum])));
}

LUA_FUNC(getNumEscortShips, l_Mission, NULL, "Number", "Gets escort ship")
{
	return lua_set_args(L, "i", hud_escort_num_ships_on_list());
}

LUA_FUNC(getEscortShip, l_Mission, "Escort index", "Ship object", "Gets escort ship")
{
	int idx;
	if(!lua_get_args(L, "i", &idx))
		return 0;

	if(idx < 1)
		return LUA_RETURN_NIL;

	//Lua->FS2
	idx--;

	idx = hud_escort_return_objnum(idx);
	
	if(idx < 0)
		return LUA_RETURN_NIL;

	return lua_set_args(L, "o", l_Ship.Set(object_h(&Objects[idx])));
}

//**********LIBRARY: Tables
lua_lib l_Tables("Tables", "tb", "Tables library");

LUA_FUNC(getNumShipClasses, l_Tables, NULL, "Number", "Gets number of ship classes")
{
	if(!ships_inited)
		return lua_set_args(L, "i", 0);	//No ships loaded...should be 0

	return lua_set_args(L, "i", Num_ship_classes);
}

LUA_FUNC(getShipClassByIndex, l_Tables, "Class index", "Shipclass object", "Gets ship class by index")
{
	if(!ships_inited)
		return LUA_RETURN_NIL;

	int idx;
	if(!lua_get_args(L, "i", &idx))
		return 0;

	//Lua->FS2
	idx--;
	
	if(idx < 0 || idx > Num_ship_classes) {
		return LUA_RETURN_NIL;
	}

	return lua_set_args(L, "o", l_Shipclass.Set(idx));
}

LUA_FUNC(getShipClass, l_Tables, "Class name", "Shipclass object", "Gets ship class")
{
	if(!ships_inited)
		return LUA_RETURN_NIL;

	char *name;
	if(!lua_get_args(L, "s", &name))
		return 0;

	int idx = ship_info_lookup(name);
	
	if(idx < 0) {
		return LUA_RETURN_NIL;
	}

	return lua_set_args(L, "o", l_Shipclass.Set(idx));
}

//**********LIBRARY: Keyboard
/*lua_lib l_Keyboard("kb", "Keyboard library");
//WMC - For some reason, this always returns true
LUA_FUNC(isKeyPressed, l_Keyboard, "Letter", "True if key is pressed, false if not", "Determines whether the given ASCII key is pressed. (If a string is given, only the first character is used)")
{
	char *s;
	if(!lua_get_args(L, "s", &s))
		return LUA_RETURN_NIL;

	char c = s[0];

	if(c == key_to_ascii(key_inkey()))
		return LUA_RETURN_TRUE;
	else
		return LUA_RETURN_FALSE;
}*/

//**********LIBRARY: Mouse
lua_lib l_Mouse("Mouse", "ms", "Mouse library");

extern int mouse_inited;

LUA_FUNC(getX, l_Mouse, "[Unscale]", "X pos (Number)", "Gets Mouse X pos")
{
	if(!mouse_inited)
		return LUA_RETURN_NIL;

	int x;
	bool u = false;
	lua_get_args(L, "|b", &u);

	if(u)
		mouse_get_pos_unscaled(&x, NULL);
	else
		mouse_get_pos(&x, NULL);

	return lua_set_args(L, "i", x);
}

LUA_FUNC(getY, l_Mouse, "[Unscale]", "Y pos (Number)", "Gets Mouse Y pos")
{
	if(!mouse_inited)
		return LUA_RETURN_NIL;

	int y;
	bool u = false;
	lua_get_args(L, "|b", &u);

	if(u)
		mouse_get_pos_unscaled(NULL, &y);
	else
		mouse_get_pos(NULL, &y);

	return lua_set_args(L, "i", y);
}

LUA_FUNC(isButtonDown, l_Mouse, "{Left, Right, or Middle}, [...], [...]", "Whether specified buttons are pressed (Boolean)", "Returns whether the specified mouse buttons are up or down")
{
	if(!mouse_inited)
		return LUA_RETURN_NIL;

	char *sa[3] = {NULL, NULL, NULL};
	lua_get_args(L, "s|ss", &sa[0], &sa[1], &sa[2]);	//Like a snake!

	bool rtn = false;
	int check_flags = 0;

	for(int i = 0; i < 3; i++)
	{
		if(sa[i] == NULL)
			break;

		if(!stricmp(sa[i], "Left"))
			check_flags |= MOUSE_LEFT_BUTTON;
		else if(!stricmp(sa[i], "Middle"))
			check_flags |= MOUSE_MIDDLE_BUTTON;
		else if(!stricmp(sa[i], "Right"))
			check_flags |= MOUSE_RIGHT_BUTTON;
	}

	if(mouse_down(check_flags))
		rtn = true;

	return lua_set_args(L, "b", rtn);
}

LUA_FUNC(setCursorImage, l_Mouse, "Image filename w/o extension, [Lock,Unlock]", "Y pos (Number)", "Sets mouse cursor image, and allows you to lock/unlock the image. (A locked cursor may only be changed with the unlock parameter)")
{
	if(!mouse_inited || !Gr_inited)
		return LUA_RETURN_NIL;

	char *s = NULL;
	char *u = NULL;
	if(!lua_get_args(L, "s|s", &s, &u))
		return LUA_RETURN_NIL;

	int ul = 0;
	if(u != NULL)
	{
		if(!stricmp("Lock", u))
			ul = GR_CURSOR_LOCK;
		else if(!stricmp("Unlock", u))
			ul = GR_CURSOR_UNLOCK;
	}

	gr_set_cursor_bitmap(bm_load(s), ul);

	return LUA_RETURN_NIL;
}

LUA_FUNC(setMouseHidden, l_Mouse, "True to hide mouse, false to show it", NULL, "Shows or hides mouse cursor")
{
	if(!mouse_inited)
		return LUA_RETURN_NIL;

	bool b = false;
	lua_get_args(L, "b", &b);

	if(b)
		Mouse_hidden = 1;
	else
		Mouse_hidden = 0;

	return LUA_RETURN_NIL;
}

//**********LIBRARY: Graphics
lua_lib l_Graphics("Graphics", "gr", "Graphics Library");

LUA_FUNC(clearScreen, l_Graphics, "[Red], [Green], [Blue]", NULL, "Clears the screen to black, or the color specified.")
{
	int r,g,b;
	r=g=b=0;
	lua_get_args(L, "|iii", &r, &g, &b);

	//WMC - Set to valid values
	if(r != 0 || g != 0 || b != 0)
	{
		CAP(r,0,255);
		CAP(g,0,255);
		CAP(b,0,255);
		gr_set_clear_color(r,g,b);
		gr_clear();
		gr_set_clear_color(0,0,0);

		return LUA_RETURN_NIL;
	}

	gr_clear();
	return LUA_RETURN_NIL;
}

LUA_FUNC(getScreenWidth, l_Graphics, NULL, "Width in pixels (Number)", "Gets screen width")
{
	if(!Gr_inited)
		return LUA_RETURN_NIL;

	return lua_set_args(L, "i", gr_screen.max_w);
}

LUA_FUNC(getScreenHeight, l_Graphics, NULL, "Height in pixels (Number)", "Gets screen height")
{
	if(!Gr_inited)
		return LUA_RETURN_NIL;

	return lua_set_args(L, "i", gr_screen.max_h);
}

#define MAX_TEXT_LINES		256

LUA_FUNC(drawString, l_Graphics, "String, x1, y1, [Resize], [x2], [y2]", NULL, "Draws a string")
{
	if(!Gr_inited)
		return LUA_RETURN_NIL;

	int x,y;
	char *s;
	int x2=-1,y2=-1;
	bool r=true;

	if(!lua_get_args(L, "sii|bii", &s, &x, &y, &r, &x2, &y2))
		return LUA_RETURN_NIL;

	if(x2 < 0) {
		gr_string(x,y,s,r);
	}
	else
	{
		int *linelengths = new int[MAX_TEXT_LINES];
		char **linestarts = new char*[MAX_TEXT_LINES];

		int num_lines = split_str(s, x2-x, linelengths, linestarts, MAX_TEXT_LINES);

		//Make sure we don't go over size
		int line_ht = gr_get_font_height();
		y2 = line_ht * (y2-y);
		if(y2 < num_lines)
			num_lines = y2;

		y2 = y;

		char rep;
		char *reptr;
		for(int i = 0; i < num_lines; i++)
		{
			//Increment line height
			y2 += line_ht;
			//WMC - rather than make a new string each line, set the right character to null
			reptr = &linestarts[i][linelengths[i]];
			rep = *reptr;
			*reptr = '\0';

			//Draw the string
			gr_string(x,y2,linestarts[i],r);

			//Set character back
			*reptr = rep;
		}

		delete[] linelengths;
		delete[] linestarts;
	}

	return LUA_RETURN_NIL;
}

LUA_FUNC(getTextWidth, l_Graphics, "Text to get width of", "Text width", "Gets text width")
{
	if(!Gr_inited)
		return LUA_RETURN_NIL;

	char *s;
	if(!lua_get_args(L, "s", &s))
		return LUA_RETURN_NIL;

	int w;

	gr_get_string_size(&w, NULL, s);
	
	return lua_set_args(L, "i", w);
}

LUA_FUNC(getTextHeight, l_Graphics, NULL, "Text height", "Gets current font's height")
{
	if(!Gr_inited)
		return LUA_RETURN_NIL;
	
	return lua_set_args(L, "i", gr_get_font_height());
}

LUA_FUNC(setColor, l_Graphics, "Red, Green, Blue, [alpha]", NULL, "Sets 2D drawing color")
{
	if(!Gr_inited)
		return LUA_RETURN_NIL;

	int r,g,b,a=255;

	if(!lua_get_args(L, "iii|i", &r, &g, &b, &a))
		return LUA_RETURN_NIL;

	color ac;
	gr_init_alphacolor(&ac,r,g,b,a);
	gr_set_color_fast(&ac);

	return 0;
}

LUA_FUNC(setFont, l_Graphics, "Font index", NULL, "Sets current font")
{
	if(!Gr_inited)
		return LUA_RETURN_NIL;

	int fn;

	if(!lua_get_args(L, "i", &fn))
		return LUA_RETURN_NIL;

	//Lua->FS2
	fn--;

	gr_set_font(fn);

	return 0;
}

LUA_FUNC(drawPixel, l_Graphics, "x, y, [Resize]", NULL, "Sets pixel to current color")
{
	if(!Gr_inited)
		return LUA_RETURN_NIL;

	int x,y;
	bool r=true;

	if(!lua_get_args(L, "ii|b", &x, &y, &r))
		return LUA_RETURN_NIL;

	gr_pixel(x,y,r);

	return LUA_RETURN_NIL;
}

LUA_FUNC(drawLine, l_Graphics, "x1, y1, x2, y2, [Resize]", NULL, "Draws a line with the current color")
{
	if(!Gr_inited)
		return LUA_RETURN_NIL;

	int x1,y1,x2,y2;
	bool r=true;

	if(!lua_get_args(L, "iiii|b", &x1, &y1, &x2, &y2, &r))
		return LUA_RETURN_NIL;

	gr_line(x1,y1,x2,y2,r);

	return LUA_RETURN_NIL;
}

LUA_FUNC(drawRectangle, l_Graphics, "x1, y1, x2, y2, [Filled], [Resize]", NULL, "Draws a rectangle with the current color; default is filled")
{
	if(!Gr_inited)
		return LUA_RETURN_NIL;

	int x1,y1,x2,y2;
	bool f=true;
	bool r=true;

	if(!lua_get_args(L, "iiii|bb", &x1, &y1, &x2, &y2, &f, &r))
		return LUA_RETURN_NIL;

	if(f)
	{
		gr_rect(x1, y1, x2-x1, y2-y1, r);
	}
	else
	{
		gr_line(x1,y1,x2,y1,r);	//Top
		gr_line(x1,y2,x2,y2,r); //Bottom
		gr_line(x1,y1,x1,y2,r);	//Left
		gr_line(x2,y1,x2,y2,r);	//Right
	}

	return LUA_RETURN_NIL;
}

LUA_FUNC(drawGradientLine, l_Graphics, "x1, y1, x2, y2, [Resize]", NULL, "Draws a line that steadily fades out")
{
	if(!Gr_inited)
		return 0;

	int x1,y1,x2,y2;
	bool r=true;

	if(!lua_get_args(L, "iiii|b", &x1, &y1, &x2, &y2, &r))
		return LUA_RETURN_NIL;

	gr_gradient(x1,y1,x2,y2,r);

	return LUA_RETURN_NIL;
}

LUA_FUNC(drawCircle, l_Graphics, "Radius, x, y, [Resize]", NULL, "Draws a circle")
{
	if(!Gr_inited)
		return LUA_RETURN_NIL;

	int x,y,ra;
	bool r=true;

	if(!lua_get_args(L, "iii|b", &ra,&x,&y,&r))
		return LUA_RETURN_NIL;

	gr_circle(x,y, ra, r);

	return LUA_RETURN_NIL;
}

LUA_FUNC(drawCurve, l_Graphics, "x, y, Radius, Direction", NULL, "Draws a curve")
{
	if(!Gr_inited)
		return LUA_RETURN_NIL;

	int x,y,ra,d;

	if(!lua_get_args(L, "iiii|b", &x,&y,&ra,&d))
		return LUA_RETURN_NIL;

	gr_curve(x,y,ra,d);

	return LUA_RETURN_NIL;
}

LUA_FUNC(drawMonochromeImage, l_Graphics, "Image name, x, y, [Resize], [Width to show], [Height to show], [X start], [Y start], [Mirror]", "Whether image was drawn", "Draws a monochrome image using the current color")
{
	if(!Gr_inited)
		return LUA_RETURN_NIL;

	int x,y;
	int rw,rh;
	char *s;
	int w=0;
	int h=0;
	int sx=0;
	int sy=0;
	bool r = true;
	bool m = false;

	if(!lua_get_args(L, "sii|biiiib", &s,&x,&y,&r,&w,&h,&sx,&sy,&m))
		return LUA_RETURN_NIL;

	int idx = bm_load(s);

	if(idx < 0)
		return lua_set_args(L, "b", false);

	bm_get_info(idx, &rw, &rh);

	if(w==0)
		w = rw;
	if(h==0)
		h = rw;

	if(sx < 0)
		sx = rw + sx;

	if(sy < 0)
		sy = rh + sy;

	gr_set_bitmap(idx);
	gr_aabitmap_ex(x, y, w, h, sx, sy, r, m);

	return lua_set_args(L, "b", true);
}

LUA_FUNC(drawImage, l_Graphics, "Image name, x, y, [Resize], [Width to show], [Height to show], [X start], [Y start]", "Whether image was drawn", "Draws an image")
{
	if(!Gr_inited)
		return LUA_RETURN_NIL;

	int x,y;
	int rw,rh;
	char *s;
	int w=0;
	int h=0;
	int sx=0;
	int sy=0;
	bool r;

	if(!lua_get_args(L, "sii|biiii", &s,&x,&y,&r,&w,&h,&sx,&sy))
		return LUA_RETURN_NIL;

	int idx = bm_load(s);

	if(idx < 0)
		return lua_set_args(L, "b", false);

	bm_get_info(idx, &rw, &rh);

	if(w==0)
		w = rw;
	if(h==0)
		h = rw;

	if(sx < 0)
		sx = rw + sx;

	if(sy < 0)
		sy = rh + sy;

	gr_set_bitmap(idx);
	gr_bitmap_ex(x, y, w, h, sx, sy);

	return lua_set_args(L, "b", true);
}

LUA_FUNC(getImageWidth, l_Graphics, "Image name, [Resize]", "Image width", "Gets image width")
{
	char *s;
	bool b = true;
	if(!lua_get_args(L, "s|b", &s, &b))
		return LUA_RETURN_NIL;

	int w;
	
	int idx = bm_load(s);

	if(idx < 0)
		return LUA_RETURN_NIL;

	bm_get_info(idx, &w);
	return lua_set_args(L, "i", w);
}

LUA_FUNC(getImageHeight, l_Graphics, "Image name, [Resize]", "Image height", "Gets image height")
{
	char *s;
	bool b = true;
	if(!lua_get_args(L, "s|b", &s, &b))
		return LUA_RETURN_NIL;

	int h;
	
	int idx = bm_load(s);

	if(idx < 0)
		return LUA_RETURN_NIL;

	bm_get_info(idx, NULL, &h);
	return lua_set_args(L, "i", h);
}

LUA_FUNC(flashScreen, l_Graphics, "Red, Green, Blue", NULL, "Flashes the screen")
{
	if(!Gr_inited)
		return LUA_RETURN_NIL;

	int r,g,b;

	if(!lua_get_args(L, "iii", &r, &g, &b))
		return LUA_RETURN_NIL;

	gr_flash(r,g,b);

	return LUA_RETURN_NIL;
}

//**********LIBRARY: Sound
lua_lib l_SoundLib("Sound", "sd", "Sound Library");

LUA_FUNC(playGameSound, l_SoundLib, "Sound filename, [Panning (-1.0 left to 1.0 right)], [Volume %], [Priority 0-3] [Voice Message?]", "True if sound was played, false if not (Replaced with a sound instance object in the future)", "Plays a sound from #Game Sounds in sounds.tbl. A priority of 0 indicates that the song must play; 1-3 will specify the maximum number of that sound that can be played")
{
	char *s;
	float pan=0.0f;
	float vol=100.0f;
	int pri=0;
	bool voice_msg = false;
	if(!lua_get_args(L, "s|ffib", &s, &pan, &vol, &pri, &voice_msg))
		return LUA_RETURN_NIL;

	int idx = gamesnd_get_by_name(s);
	
	if(idx < 0)
		return LUA_RETURN_FALSE;

	if(pri < 0 || pri > 3)
		pri = 0;

	if(pan < -1.0f)
		pan = -1.0f;
	if(pan > 1.0f)
		pan = 1.0f;
	if(vol < 0.0f)
		vol = 0.0f;
	if(vol > 100.0f)
		vol = 100.0f;

	idx = snd_play(&Snds[idx], pan, vol*0.01f, pri, voice_msg);

	return lua_set_args(L, "b", idx > -1);
}

LUA_FUNC(playInterfaceSound, l_SoundLib, "Sound filename", "True if sound was played, false if not", "Plays a sound from #Interface Sounds in sounds.tbl")
{
	char *s;
	if(!lua_get_args(L, "s", &s))
		return LUA_RETURN_NIL;

	int idx;
	for(idx = 0; idx < Num_iface_sounds; idx++)
	{
		if(!stricmp(Snds_iface[idx].filename, s))
			break;
	}
	
	if(idx == Num_iface_sounds)
		return LUA_RETURN_FALSE;

	gamesnd_play_iface(idx);

	return lua_set_args(L, "b", idx > -1);
}

// *************************Housekeeping*************************

void lua_add_vars(lua_State *L, int table_loc, lua_lib_h *lib, lua_var_hh *var, lua_var_hh *var_end)
{
	//We have variables
	if(var != var_end || lib->Functions.size())
	{
		//Set __index to special handler
		lua_pushstring(L, "__index");
		lua_pushboolean(L, 0);	//Push boolean argument to tell index_handler we are "get"
		lua_pushstring(L, "__index");
		lua_pushcclosure(L, lua_index_handler, 2);
		lua_settable(L, table_loc);

		lua_pushstring(L, "__newindex");
		lua_pushboolean(L, 1);	//Push boolean argument to tell index_handler we are "set"
		lua_pushstring(L, "__newindex");
		lua_pushcclosure(L, lua_index_handler, 2);
		lua_settable(L, table_loc);
	}

	std::string str;
	//Add variables
	for(; var < var_end; var++)
	{
		//Set a bogus value here so index_handler knows it's there
		//and not a typo
		if(!var->IsArray)
		{
			lua_pushstring(L, var->Name);
			lua_pushnumber(L, (lua_Number)INDEX_HANDLER_VAR_TRIGGER);
			lua_settable(L, table_loc);

			//Set function
			str = "var";
			str += var->Name;
			lua_pushstring(L, str.c_str());
			lua_pushboolean(L, 0);	//Default is get
			lua_pushstring(L, str.c_str());
			lua_pushcclosure(L, var->Function, 2);
			lua_settable(L, table_loc);
		}
		else
		{
			//WMC - Bleh. This doesn't work.
			//The table is set properly, but for some reason
			//Lua doesn't call the indexing functions
			//An array has its own metatable
			lua_newtable(L);

			//Set it for object metatable
			lua_pushstring(L, var->Name);
			lua_pushvalue(L, -2);
			lua_rawset(L, table_loc);

			//Set the metatable for the array to itself(?)
			lua_pushstring(L, "__metatable");
			lua_pushvalue(L, -2);
			lua_rawset(L, -3);

			//Index (get) function has upvalue of 0
			lua_pushstring(L, "__index");
			lua_pushboolean(L, 0);
			lua_pushstring(L, "__index");
			lua_pushcclosure(L, var->Function, 2);
			lua_rawset(L, -3);

			//Index (set) function has upvalue of 1
			lua_pushstring(L, "__newindex");
			lua_pushboolean(L, 1);
			lua_pushstring(L, "__newindex");
			lua_pushcclosure(L, var->Function, 2);
			lua_rawset(L, -3);

			//DEBUG:
/*			lua_pushstring(L, var->Name);
			lua_gettable(L, table_loc);
			lua_pushstring(L, "__index");
			lua_gettable(L, -2);

			char buf[10240] = {0};
			lua_stackdump(L, buf);
			Error(LOCATION, buf);
			lua_Debug ar;
			lua_getstack(L, 0, &ar);
			lua_getinfo(L, ">nl", &ar);*/

			//Set array metatable
			//lua_settable(L, table_loc);
		}
	}

	//Set the indexer
	if(lib->Indexer != NULL)
	{
		//If we are using the index handler, put it in its special spot
		if(var != var_end || lib->Functions.size())
		{
			lua_pushstring(L, "__indexer");
			lua_pushboolean(L, 0);	//Default is get
			lua_pushcclosure(L, lib->Indexer, 1);
			lua_settable(L, table_loc);
		}
		else
		{
			//Otherwise, we have to set the indexer up
			//using Lua's normal __index variables
			lua_pushstring(L, "__index");
			lua_pushboolean(L, 0);
			lua_pushstring(L, "__index");
			lua_pushcclosure(L, lib->Indexer, 2);
			lua_settable(L, table_loc);

			lua_pushstring(L, "__newindex");
			lua_pushboolean(L, 1);
			lua_pushstring(L, "__newindex");
			lua_pushcclosure(L, lib->Indexer, 2);
			lua_settable(L, table_loc);
		}
	}
}

#endif //USE_LUA

//Inits LUA
//Note that "libraries" must end with a {NULL, NULL}
//element
int script_state::CreateLuaState()
{
#ifdef USE_LUA
	lua_State *L = lua_open();   /* opens Lua */
    luaopen_base(L);             /* opens the basic library */
    luaopen_table(L);            /* opens the table library */

	//LUAJIT hates io :(
	luaopen_io(L);               /* opens the I/O library */

    luaopen_string(L);           /* opens the string lib. */
    luaopen_math(L);             /* opens the math lib. */

	if(L == NULL)
	{
		Warning(LOCATION, "Could not initialize Lua");
		return 0;
	}

	lua_lib_h *lib = &lua_Libraries[0];
	lua_lib_h *lib_end = &lua_Libraries[lua_Libraries.size()];
	lua_lib_h *obj = &lua_Objects[0];
	lua_lib_h *obj_end = &lua_Objects[lua_Objects.size()];
	lua_lib_h *libobj = NULL;
	lua_lib_h *libobj_end = &lua_Libraries[lua_Libraries.size()];
	lua_func_hh *func;
	lua_func_hh *func_end;
	lua_var_hh *var;
	lua_var_hh *var_end;
	int i;	//used later

	//*****CHECK FOR BAD THINGS
#ifndef NDEBUG
	lua_func_hh *ofunc;
	lua_func_hh *ofunc_end;

	//Global functions/libraries/objects
	mprintf(("LUA: Performing global function/(library/object) name repeat check...\n"));
	lib = &lua_Libraries[0];
	libobj = &lua_Libraries[0];
	for(; libobj < libobj_end; libobj++)
	{
		if(libobj->Name != NULL && strlen(lib->Name))
			continue;
		
		func = &libobj->Functions[0];
		func_end = &libobj->Functions[libobj->Functions.size()];

		for(; func < func_end; func++)
		{
			for(lib = &lua_Libraries[0]; lib < lib_end; lib++)
			{
				if(!stricmp(func->Name, lib->Name))
					Error(LOCATION, "Lua global function '%s' has the name as library '%s'. Get a coder.", func->Name, lib->Name);
				if(!stricmp(func->Name, lib->ShortName))
					Error(LOCATION, "Lua global function '%s' has the name as library '%s (%s)' shortname. Get a coder.", func->Name, lib->Name, lib->ShortName);

				if(lib->Name == NULL || !strlen(lib->Name))
				{
					ofunc = &lib->Functions[0];
					ofunc_end = &lib->Functions[lib->Functions.size()];
					for(; ofunc < func_end; func++)
					{
						if(func == ofunc)
							continue;

						if(!stricmp(func->Name, ofunc->Name))
							Error(LOCATION, "Global function '%s' in lib '%s' and global function '%s' in lib '%s' have the same name.", func->Name, libobj->Name, ofunc->Name, lib->Name);
					}
				}
			}

			for(obj = &lua_Objects[0]; obj < obj_end; obj++) {
				if(!stricmp(func->Name, obj->Name))
					Error(LOCATION, "Lua global function '%s' and object '%s' have the same name. Get a coder.", func->Name, obj->Name);
			}
		}
	}

	//Libraries/objects
	mprintf(("LUA: Performing library/object name repeat check...\n"));
	for(lib = &lua_Libraries[0]; lib < lib_end; lib++)
	{
		for(obj = &lua_Objects[0]; obj < obj_end; obj++)
		{
			if(!stricmp(lib->Name, obj->Name))
				Error(LOCATION, "Lua library '%s' and object '%s' have the same name. Get a coder.", lib->Name, obj->Name);
			if(!stricmp(lib->ShortName, obj->Name))
				Error(LOCATION, "Lua library '%s (%s)' has the same shortname as object name '%s'. Get a coder.", lib->Name, lib->ShortName, obj->Name);
		}
	}

	//Do double-object check
	mprintf(("LUA: Performing object/object name repeat check...\n"));
	for(obj = &lua_Objects[0]; obj < obj_end; obj++)
	{
		for(libobj = obj+1; libobj < obj_end; libobj++)
		{
			if(!stricmp(obj->Name, libobj->Name))
				Error(LOCATION, "Lua object '%s' and object '%s' have the same name. Get a coder.", obj->Name, libobj->Name);
		}

		//Check for duplicate functions within objects
		func = &obj->Functions[0];
		func_end = &obj->Functions[obj->Functions.size()];
		for(; func < func_end; func++) {
			ofunc = func+1;
			for(; ofunc < func_end; ofunc++) {
				if(!stricmp(func->Name, ofunc->Name))
					Error(LOCATION, "Function '%s' and function '%s' have the same name within object '%s'. Get a coder.", func->Name, ofunc->Name, obj->Name);
			}
		}
	}

	//Do lib-on-lib check
	mprintf(("LUA: Performing library/library name repeat check...\n"));
	for(lib = &lua_Libraries[0]; lib < lib_end; lib++)
	{
		for(libobj = lib+1; libobj < lib_end; libobj++) {
			if(!stricmp(lib->Name, libobj->Name)
				|| !stricmp(lib->ShortName, libobj->ShortName)
				|| !stricmp(lib->Name, libobj->ShortName)
				|| !stricmp(lib->ShortName, libobj->ShortName))
				Error(LOCATION, "Lua library '%s (%s)' and library '%s (%s)' have the same name or shortname. Get a coder.", lib->Name, lib->ShortName, libobj->Name, libobj->ShortName);
		}

		//Check for duplicate functions within libs
		func = &lib->Functions[0];
		func_end = &lib->Functions[lib->Functions.size()];
		for(; func < func_end; func++)
		{
			for(ofunc = func+1; ofunc < func_end; ofunc++) {
				if(!stricmp(func->Name, ofunc->Name))
					Error(LOCATION, "Function '%s' and function '%s' have the same name within library '%s'. Get a coder.", func->Name, ofunc->Name, lib->Name);
			}
		}
	}
#endif

	//*****INITIALIZE ALL LIBRARY FUNCTIONS
	mprintf(("LUA: Initializing library functions...\n"));
	int table_loc;
	lib = &lua_Libraries[0];
	lib_end = &lua_Libraries[lua_Libraries.size()];
	for(; lib < lib_end; lib++)
	{
		//If a library name is given, register functions as library items
		//If not, register functions as globals
		if(lib->Name != NULL && strlen(lib->Name))
		{
			//Register library functions
			//luaL_register(L, lib->library_name, lib->library_funcs);
			//luaL_openlib(L, lib->library_name, lib->library_funcs, 0);
				
			//NOTE FROM WMC:
			//The following is based on luaL_openlib from lauxlib.c
			//The default library can't be used because my custom script
			//function array features a field for function description

			//Check for the library's existence
			lua_pushstring(L, lib->Name);
			lua_gettable(L, LUA_GLOBALSINDEX);

			//If it doesn't exist...
			if (lua_isnil(L, -1))
			{
				lua_pop(L, 1);									//Pop the nil resultfrom the stack
				lua_newtable(L);								//Create a new table
				lua_pushstring(L, lib->Name);					//Add a string to the stack
				lua_pushvalue(L, -2);							//Push the table
				lua_settable(L, LUA_GLOBALSINDEX);				//Register the table with the new name
				lua_pushstring(L, lib->ShortName);				//Add short name string to the stack
				lua_pushvalue(L, -2);							//Push the table again
				lua_settable(L, LUA_GLOBALSINDEX);				//Register the table with the short name
			}

			table_loc = lua_gettop(L);

			func = &lib->Functions[0];
			func_end = &lib->Functions[lib->Functions.size()];
			for(; func < func_end; func++)
			{
				Assert(func->Name != NULL && strlen(func->Name));
				if(func->Function == NULL)
					continue;

				//Add each function
				lua_pushstring(L, func->Name);				//Push the function's name onto the stack
				lua_pushstring(L, func->Name);				//Push upvalue
				lua_pushcclosure(L, func->Function, 1);		//Push the function pointer onto the stack
				lua_settable(L, -3);						//Add it into the current lib table
			}

			lua_add_vars(L, table_loc, lib, &lib->Variables[0], &lib->Variables[lib->Variables.size()]);
		}
		else
		{
			//Iterate through the function list
			func = &lib->Functions[0];
			func_end = &lib->Functions[lib->Functions.size()];
			for(; func < func_end; func++)
			{
				//Sanity checking
				Assert(func->Name != NULL && strlen(func->Name));
				if(func->Function == NULL)
					continue;

				//Register the function with the name given as a global
				lua_pushstring(L, func->Name);
				lua_pushstring(L, func->Name);
				lua_pushcclosure(L, func->Function, 1);
				lua_settable(L, LUA_GLOBALSINDEX);
			}

			lua_add_vars(L, LUA_GLOBALSINDEX, lib, &lib->Variables[0], &lib->Variables[lib->Variables.size()]);
		}

		//Handle objects and their methods in a library
	}

	//*****INITIALIZE OBJECT FUNCTIONS
	mprintf(("LUA: Initializing object functions...\n"));
	lib = &lua_Objects[0];
	lib_end = &lua_Objects[lua_Objects.size()];
	std::string str;
	for(; lib < lib_end; lib++)
	{
		if(!luaL_newmetatable(L, lib->Name))
		{
			Warning(LOCATION, "Couldn't create metatable for object '%s'", lib->Name);
			continue;
		}
		//Get the absolute position of the object metatable for later use
		table_loc = lua_gettop(L);

		//***Add the functions into the metatables
		//Because both the [] operator and function list share the "__index"
		//entry in the metatable, we must check for both and give an error
		//to be safe
		bool index_oper_already = false;
		bool index_meth_already = false;
		bool concat_oper_already = false;

		//WMC - This is a bit odd. Basically, to handle derivatives, I have a double-loop set up.
		lua_lib_h *clib = lib;
		for(i = 0; i < 2; i++)
		{
			if(i==0)
			{
				if(lib->Derivator > -1)
					clib = &lua_Objects[lib->Derivator];
				else
					continue;
			}
			else
			{
				clib = lib;
			}

			func = &clib->Functions[0];
			func_end = &clib->Functions[clib->Functions.size()];
			var = &clib->Variables[0];
			var_end = &clib->Variables[clib->Variables.size()];

			for(; func < func_end; func++)
			{
				Assert(func->Name != NULL && strlen(func->Name));
				if(func->Function == NULL)
					continue;

				//WMC - First, do operator functions
				if(!strnicmp(func->Name, "__", 2))
				{
					if(!stricmp(func->Name, "__index"))
					{
						if(!index_meth_already){
							index_oper_already = true;
						} else {
							Error(LOCATION, "Attempt to set both an indexing operator and methods for Lua class '%s'; get a coder", clib->Name);
						}
					}
					lua_pushstring(L, func->Name);
					lua_pushstring(L, func->Name);		//WMC - push upvalue for debugging/warnings
					lua_pushcclosure(L, func->Function, 1);
					lua_settable(L, table_loc);
				}
				else	//This is an object method
				{
					if(index_oper_already) {
						Error(LOCATION, "Attempt to set both an indexing operator and methods for Lua class '%s'; get a coder", clib->Name);
					}

					if(!index_meth_already)
					{
						//Create the metatable
						lua_pushstring(L, "__index");
						lua_pushvalue(L, table_loc);  // pushes the metatable
						lua_settable(L, table_loc);  // metatable.__index = metatable
						index_meth_already = true;
					}
					lua_pushstring(L, func->Name);
					lua_pushstring(L, func->Name);	//WMC - push upvalue
					lua_pushcclosure(L, func->Function, 1);
					lua_settable(L, -3);
				}
			}

			lua_add_vars(L, table_loc, lib, var, var_end);
		}

		//Add concat operator if necessary
		if(!concat_oper_already)
		{
			lua_pushstring(L, "__concat");
			lua_pushstring(L, "__concat");		//WMC - push upvalue for debugging/warnings
			lua_pushcclosure(L, lua_concat_handler, 1);
			lua_settable(L, table_loc);
		}
	}
	SetLuaSession(L);

	return 1;
#else
	return 0;
#endif
}

#ifdef USE_LUA
void output_lib_meta(FILE *fp, lua_lib_h *main_lib, lua_lib_h *lib_deriv)
{
	lua_func_hh *func, *func_end;
	lua_var_hh *var, *var_end;
	int i,j;
	bool draw_header;
	fputs("<dd><dl>", fp);
	lua_lib_h *lib = main_lib;

	//Operators
	draw_header = false;
	if(main_lib->Functions.size() || (lib_deriv != NULL && lib_deriv->Functions.size()))
	{
		for(i = 0; i < 2; i++)
		{
			func = &lib->Functions[0];
			func_end = &lib->Functions[lib->Functions.size()];
			for(; func < func_end; func++)
			{
				if(!strncmp(func->Name, "__", 2)) {
					draw_header = true;
					break;
				}
			}
			if(lib_deriv == NULL)
				break;

			lib = lib_deriv;
		}
	}

	lib = main_lib;
	if(draw_header) {
		fputs("<dt><h3>Operators</h3></dt><dd><dl>", fp);
	}
	for(i = 0; i < 2; i++)
	{
		func = &lib->Functions[0];
		func_end = &lib->Functions[lib->Functions.size()];

		for(; func < func_end; func++)
		{
			if(strncmp(func->Name, "__", 2)) {
				continue;
			}

			char *draw_name = func->Name;
			for(j = 0; j < lua_Num_operators; j++)
			{
				if(!stricmp(draw_name, lua_Operators[j].src)) {
					draw_name = lua_Operators[j].dest;
					break;
				}
			}

			if(func->Arguments != NULL) {
				fprintf(fp, "<dt><b>%s - <i>%s</i></b></dt>", draw_name, func->Arguments);
			} else {
				fprintf(fp, "<dt><b>%s</b></dt>", draw_name);
			}

			if(func->Description != NULL) {
				fprintf(fp, "<dd>%s</dd>", func->Description);
			} else {
				fputs("<dd>No description</dd>", fp);
			}

			if(func->ReturnValues != NULL) {
				fprintf(fp, "<dd><b>Return values:</b> %s<br>&nbsp;</dd>", func->ReturnValues);
			} else {
				fputs("<dd><b>Return values:</b> None<br>&nbsp;</dd>", fp);
			}
		}

		if(lib_deriv == NULL)
			break;

		lib = lib_deriv;
	}

	//Variables
	if(main_lib->Variables.size() || (lib_deriv != NULL && lib_deriv->Variables.size()))
		draw_header = true;
	else draw_header = false;
	lib = main_lib;
	if(draw_header) {
		fputs("<dt><h3>Variables</h3></dt><dd><dl>", fp);
	}
	for(i = 0; i < 2; i++)
	{
		var = &lib->Variables[0];
		var_end = &lib->Variables[lib->Variables.size()];

		for(; var < var_end; var++)
		{
			if(var->Type != NULL) {
				fprintf(fp, "<dt><i>%s</i> <b>%s</b></dt>", var->Type, var->Name);
			} else {
				fprintf(fp, "<dt><b>%s</b></dt>", var->Name);
			}

			if(var->Description != NULL) {
				fprintf(fp, "<dd>%s</dd>", var->Description);
			} else {
				fputs("<dd>No description</dd>", fp);
			}
		}

		if(lib_deriv == NULL)
			break;

		lib = lib_deriv;
	}
	if(draw_header) {
		fputs("</dl></dd>", fp);
	}

	//Functions
	if(main_lib->Functions.size() || (lib_deriv != NULL && lib_deriv->Functions.size()))
		draw_header = true;
	else draw_header = false;
	lib = main_lib;
	if(draw_header) {
		fputs("<dt><h3>Functions</h3></dt><dd><dl>", fp);
	}
	for(i = 0; i < 2; i++)
	{
		func = &lib->Functions[0];
		func_end = &lib->Functions[lib->Functions.size()];

		for(; func < func_end; func++)
		{
			if(!strncmp(func->Name, "__", 2)) {
				continue;
			}

			if(func->Arguments != NULL) {
				fprintf(fp, "<dt><b>%s(</b><i>%s</i><b>)</b></dt>", func->Name, func->Arguments);
			} else {
				fprintf(fp, "<dt><b>%s()</b></dt>", func->Name);
			}

			if(func->Description != NULL) {
				fprintf(fp, "<dd>%s</dd>", func->Description);
			} else {
				fputs("<dd>No description</dd>", fp);
			}

			if(func->ReturnValues != NULL) {
				fprintf(fp, "<dd><b>Return values:</b> %s<br>&nbsp;</dd>", func->ReturnValues);
			} else {
				fputs("<dd><b>Return values:</b> None<br>&nbsp;</dd>", fp);
			}
		}

		if(lib_deriv == NULL)
			break;

		lib = lib_deriv;
	}

	if(draw_header) {
		fputs("</dl></dd>", fp);
	}
	fputs("<br></dl></dd>", fp);
}
#endif

void script_state::OutputLuaMeta(FILE *fp)
{
#ifdef USE_LUA
	//***Output Libraries
	fputs("<dl>", fp);
	fputs("<dt>Libraries</dt>", fp);
	lua_lib_h *lib = &lua_Libraries[0];
	lua_lib_h *lib_end = &lua_Libraries[lua_Libraries.size()];
	for(; lib < lib_end; lib++)
	{
		fprintf(fp, "<dd><a href=\"#%s\">%s (%s)</a> - %s</dd>", lib->Name, lib->Name, lib->ShortName, lib->Description);
	}

	//***Output objects
	lib = &lua_Objects[0];
	lib_end = &lua_Objects[lua_Objects.size()];
	fputs("<dt>Objects</dt>", fp);
	for(; lib < lib_end; lib++)
	{
		fprintf(fp, "<dd><a href=\"#%s\">%s</a> - %s</dd>", lib->Name, lib->Name, lib->Description);
	}
	fputs("</dl><br/><br/>", fp);

	//***Output libs
	fputs("<dl>", fp);
	lib = &lua_Libraries[0];
	lib_end = &lua_Libraries[lua_Libraries.size()];
	for(; lib < lib_end; lib++)
	{
		fprintf(fp, "<dt id=\"%s\"><h2>%s (%s)</h2></dt>", lib->Name, lib->Name, lib->ShortName);

		//Last param is something of a hack to handle lib derivs
		output_lib_meta(fp, lib, lib->Derivator > -1 ? &lua_Libraries[lib->Derivator] : NULL);
	}
	//***Output objects
	lib = &lua_Objects[0];
	lib_end = &lua_Objects[lua_Objects.size()];
	for(; lib < lib_end; lib++)
	{
		fprintf(fp, "<dt id=\"%s\"><h2>%s - %s</h2></dt>", lib->Name, lib->Name, lib->Description);

		//Last param is something of a hack to handle lib derivs
		output_lib_meta(fp, lib, lib->Derivator > -1 ? &lua_Objects[lib->Derivator] : NULL);
	}
	fputs("</dl>", fp);
#endif
}
