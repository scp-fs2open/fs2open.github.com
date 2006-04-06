#include "parse/scripting.h"
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
#include "particle/particle.h"
#include "iff_defs/iff_defs.h"
#include "camera/camera.h"

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
#define LUA_INDEXER(objlib, args, retvals, desc)			\
	static int lua_##objlib##___indexer(lua_State *L);		\
	lua_indexer_h lua_##objlib##___indexer_h(lua_##objlib##___indexer, objlib, args, retvals, desc);	\
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
	int counted_args = 0;

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

		if(nargs > total_args)
			break;

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
			case 'x':
				if(lua_isnumber(L, nargs)) {
					*va_arg(vl, fix*) = fl2f((float)lua_tonumber(L, nargs));
				} else {
					LuaError(L, "%s: Argument %d is an invalid type '%s'; number expected", funcname, nargs, lua_get_type_string(L, nargs));
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
						if(idx < 0 || (lua_Objects[idx].Derivator != od.meta && lua_Objects[od.meta].Derivator != idx)) {
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
		counted_args++;
	}
	va_end(vl);
	return counted_args;
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
			case 'b':	//WMC - Bool is actually int for GCC (Why...?)
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
			case 'x':
				lua_pushnumber(L, f2fl(va_arg(vl, fix)));
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

//lua_index_handler
//Depends on one upvalue, a boolean.
//false => __index
//true => __newindex
//When variables are set for a given library or object, this handles
//meting out control to the appropriate function.
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
	//if section, and uncomment the lua_rawget(L, -2) in the next !using_indexer if section.
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
		LuaError(L, "lua_setupvalue in lua_index_handler failed; get a coder");
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

//**********OBJECT: orientation matrix
//WMC - So matrix can use vector, I define it up here.
lua_obj<vec3d> l_Vector("vector", "Vector object");
//WMC - Due to the exorbitant times required to store matrix data,
//I initially store the matrix in this struct.
#define MH_FINE					0
#define MH_MATRIX_OUTOFDATE		1
#define MH_ANGLES_OUTOFDATE		2
struct matrix_h {
	int status;
	matrix mtx;
	angles ang;

	matrix_h(matrix *in){mtx = *in; status = MH_ANGLES_OUTOFDATE;}
	matrix_h(angles *in){ang = *in; status = MH_MATRIX_OUTOFDATE;}

	//WMC - Call these to make sure what you want
	//is up to date
	void ValidateAngles() {
		if(status == MH_ANGLES_OUTOFDATE) {
			vm_extract_angles_matrix(&ang, &mtx);
			status = MH_FINE;
		}
	}

	void ValidateMatrix() {
		if(status == MH_MATRIX_OUTOFDATE) {
			vm_angles_2_matrix(&mtx, &ang);
			status = MH_FINE;
		}
	}

	//LOOK LOOK LOOK LOOK LOOK LOOK 
	//IMPORTANT!!!:
	//LOOK LOOK LOOK LOOK LOOK LOOK 
	//Don't forget to set status appropriately when you change ang or mtx.
};
lua_obj<matrix_h> l_Matrix("orientation", "Orientation matrix object");

LUA_INDEXER(l_Matrix, "p,b,h or 0-9", "Number", "Orientation component - pitch, bank, heading, or index into 3x3 matrix (1-9)")
{
	matrix_h *mh;
	char *s = NULL;
	float newval = 0.0f;
	int numargs = lua_get_args(L, "os|f", l_Matrix.GetPtr(&mh), &s, &newval);

	if(!numargs || s[1] != '\0')
		return LUA_RETURN_NIL;

	int idx=0;
	if(s[0]=='p')
		idx = -1;
	else if(s[0]=='b')
		idx = -2;
	else if(s[0]=='h')
		idx = -3;
	else if(atoi(s))
		idx = atoi(s);

	if(idx < -3 || idx==0 || idx > 9)
		return LUA_RETURN_NIL;

	//Handle out of date stuff.
	float *val = NULL;
	if(idx < 0)
	{
		mh->ValidateAngles();

		if(idx == -1)
			val = &mh->ang.p;
		if(idx == -2)
			val = &mh->ang.b;
		if(idx == -3)
			val = &mh->ang.h;
	}
	else
	{
		mh->ValidateMatrix();

		idx--;	//Lua->FS2
		val = &mh->mtx.a1d[idx];
	}

	if(LUA_SETTING_VAR && *val != newval)
	{
		//WMC - I figure this is quicker
		//than just assuming matrix or angles is diff
		//and recalculating every time.

		if(idx < 0)
			mh->status = MH_MATRIX_OUTOFDATE;
		else
			mh->status = MH_ANGLES_OUTOFDATE;

		//Might as well put this here
		*val = newval;
	}

	return lua_set_args(L, "f", *val);
}

LUA_FUNC(transpose, l_Matrix, NULL, NULL, "Transposes matrix")
{
	matrix_h *mh;
	if(!lua_get_args(L, "o", l_Matrix.GetPtr(&mh)))
		return LUA_RETURN_NIL;

	mh->ValidateMatrix();
	vm_transpose_matrix(&mh->mtx);
	mh->status = MH_ANGLES_OUTOFDATE;

	return LUA_RETURN_NIL;
}


LUA_FUNC(rotateVector, l_Matrix, "Vector object", "Rotated vector", "Returns rotated version of given vector")
{
	matrix_h *mh;
	vec3d *v3;
	if(!lua_get_args(L, "oo", l_Matrix.GetPtr(&mh), l_Vector.GetPtr(&v3)))
		return LUA_RETURN_NIL;

	vec3d v3r;
	mh->ValidateMatrix();
	vm_vec_rotate(&v3r, v3, &mh->mtx);

	return lua_set_args(L, "o", l_Vector.Set(v3r));
}

LUA_FUNC(unrotateVector, l_Matrix, "Vector object", "Unrotated vector", "Returns unrotated version of given vector")
{
	matrix_h *mh;
	vec3d *v3;
	if(!lua_get_args(L, "oo", l_Matrix.GetPtr(&mh), l_Vector.GetPtr(&v3)))
		return LUA_RETURN_NIL;

	vec3d v3r;
	mh->ValidateMatrix();
	vm_vec_unrotate(&v3r, v3, &mh->mtx);

	return lua_set_args(L, "o", l_Vector.Set(v3r));
}

//**********OBJECT: vector
//WMC - see matrix for lua_obj def

LUA_INDEXER(l_Vector, "x,y,z or 1-3", "Vector", "Vector component")
{
	vec3d *v3;
	char *s = NULL;
	float newval = 0.0f;
	int numargs = lua_get_args(L, "os|f", l_Vector.GetPtr(&v3), &s, &newval);

	if(!numargs || s[1] != '\0')
		return LUA_RETURN_NIL;

	int idx=-1;
	if(s[0]=='x' || s[0] == '1')
		idx = 0;
	else if(s[0]=='y' || s[0] == '2')
		idx = 1;
	else if(s[0]=='z' || s[0] == '3')
		idx = 2;

	if(idx < 0 || idx > 3)
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
	vec3d *v3;
	if(!lua_get_args(L, "o", l_Vector.GetPtr(&v3)))
		return LUA_RETURN_NIL;

	char buf[32];
	sprintf(buf, "(%f,%f,%f)", v3->xyz.x, v3->xyz.y, v3->xyz.z);

	return lua_set_args(L, "s", buf);
}

LUA_FUNC(getMagnitude, l_Vector, NULL, "Magnitude", "Returns the magnitude of a vector (Total regardless of direction)")
{
	vec3d *v3;
	if(!lua_get_args(L, "o", l_Vector.GetPtr(&v3)))
		return LUA_RETURN_NIL;

	return lua_set_args(L, "f", vm_vec_mag(v3));
}

LUA_FUNC(getDistance, l_Vector, "Vector", "Distance", "Returns distance from another vector")
{
	vec3d *v3a, *v3b;
	if(!lua_get_args(L, "oo", l_Vector.GetPtr(&v3a), l_Vector.GetPtr(&v3b)))
		return LUA_RETURN_NIL;

	return lua_set_args(L, "f",vm_vec_dist(v3a, v3b));
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

//**********HANDLE: directive
lua_obj<int> l_Directive("directive", "Mission directive handle");

//**********HANDLE: directives
lua_obj<bool> l_Directives("directives", "Mission directives handle");

LUA_INDEXER(l_Directives, "Directive number", "directive handle", NULL)
{
	bool b;
	int idx;
	if(!lua_get_args(L, "o|i", l_Directives.Get(&b), &idx))
		return LUA_RETURN_NIL;

	if(idx < 1 || idx > Num_mission_events)
		return LUA_RETURN_FALSE;

	idx--;	//Lua->FS2

	return lua_set_args(L, "o", l_Directive.Set(idx));
}

//**********HANDLE: cmission
lua_obj<int> l_Cmission("cmission", "Campaign mission handle");
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

//**********HANDLE: Species
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

//**********HANDLE: Shiptype
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

//**********HANDLE: Weaponclass
lua_obj<int> l_Weaponclass("weaponclass", "Weapon class handle");

LUA_VAR(Name, l_Weaponclass, "string", "Weapon class name")
{
	int idx;
	char *s = NULL;
	if(!lua_get_args(L, "o|s", l_Weaponclass.Get(&idx), &s))
		return LUA_RETURN_NIL;

	if(idx < 0 || idx > Num_weapon_types)
		return LUA_RETURN_NIL;

	if(LUA_SETTING_VAR && s != NULL) {
		strncpy(Weapon_info[idx].name, s, sizeof(Weapon_info[idx].name)-1);
	}

	return lua_set_args(L, "s", Weapon_info[idx].name);
}

//**********HANDLE: Shipclass
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

LUA_VAR(AfterburnerFuelMax, l_Shipclass, "Number", "Afterburner fuel capacity")
{
	int idx;
	float fuel = -1.0f;
	if(!lua_get_args(L, "o|f", l_Shipclass.Get(&idx), &fuel))
		return LUA_RETURN_NIL;

	if(idx < 0 || idx > Num_ship_classes)
		return LUA_RETURN_NIL;

	if(LUA_SETTING_VAR && fuel >= 0.0f)
		Ship_info[idx].afterburner_fuel_capacity = fuel;

	return lua_set_args(L, "f", Ship_info[idx].afterburner_fuel_capacity);
}

LUA_VAR(CountermeasuresMax, l_Shipclass, "Number", "Ship class countermeasure max")
{
	int idx;
	int i = -1;
	if(!lua_get_args(L, "o|i", l_Shipclass.Get(&idx), &i))
		return LUA_RETURN_NIL;

	if(idx < 0 || idx > Num_ship_classes)
		return LUA_RETURN_NIL;

	if(LUA_SETTING_VAR && i > -1) {
		Ship_info[idx].cmeasure_max = i;
	}

	return lua_set_args(L, "i", Ship_info[idx].cmeasure_max);
}

LUA_VAR(HitpointsMax, l_Shipclass, "Number", "Ship class hitpoints")
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


LUA_FUNC(renderTechModel, l_Shipclass, "X1, Y1, X2, Y2, [Rotation %, Pitch %, Bank %, Zoom multiplier]", "Whether ship was rendered", "Draws ship model as if in techroom")
{
	int x1,y1,x2,y2;
	angles rot_angles = {0.0f, 0.0f, 40.0f};
	int idx;
	float zoom = 1.3f;
	if(!lua_get_args(L, "oiiii|ffff", l_Shipclass.Get(&idx), &x1, &y1, &x2, &y2, &rot_angles.h, &rot_angles.p, &rot_angles.b, &zoom))
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
	gr_set_clip(x1,y1,x2-x1,y2-y1,false);

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

//**********HANDLE: Shields
lua_obj<object_h> l_Shields("shields", "Shields handle");

LUA_INDEXER(l_Shields, "Front, left, right, back, or 1-4", "Number", "Gets or sets shield quadrant strength")
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
	else if(!stricmp(qd, "Front"))
		qdx = 0;
	else if(!stricmp(qd, "Left"))
		qdx = 1;
	else if(!stricmp(qd, "Right"))
		qdx = 2;
	else if(!stricmp(qd, "Back"))
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

//**********HANDLE: Team
lua_obj<int> l_Team("Team", "Team handle");

LUA_FUNC(__eq, l_Team, "team, team", "true if equal", "Checks whether two teams are the same team")
{
	int t1, t2;
	if(!lua_get_args(L, "oo", l_Team.Get(&t1), l_Team.Get(&t2)))
		return LUA_RETURN_FALSE;

	return lua_set_args(L, "b", (t1 == t2));
}

LUA_VAR(Name, l_Team, "string", "Team name")
{
	int tdx=-1;
	char *s=NULL;
	if(!lua_get_args(L, "o|s", l_Team.Get(&tdx), &s))
		return LUA_RETURN_NIL;

	if(tdx < 0 || tdx > Num_iffs)
		return LUA_RETURN_NIL;

	if(LUA_SETTING_VAR && s != NULL) {
		strncpy(Iff_info[tdx].iff_name, s, NAME_LENGTH-1);
	}

	return lua_set_args(L, "s", Iff_info[tdx].iff_name);
}

//**********HANDLE: Object
lua_obj<object_h> l_Object("object", "Object handle");
//Helper function
//Returns 1 if object sig stored in idx exists, and stores Objects[] index in idx
//Returns 0 if object sig does not exist, and does not change idx

LUA_VAR(Position, l_Object, "World vector", "Object world position")
{
	object_h *objh;
	vec3d *v3=NULL;
	if(!lua_get_args(L, "o|o", l_Object.GetPtr(&objh), l_Vector.GetPtr(&v3)))
		return LUA_RETURN_NIL;

	if(!objh->IsValid())
		return LUA_RETURN_NIL;

	if(LUA_SETTING_VAR && v3 != NULL) {
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

	if(!objh->IsValid())
		return LUA_RETURN_NIL;

	if(LUA_SETTING_VAR && v3 != NULL) {
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

	if(!objh->IsValid())
		return LUA_RETURN_NIL;

	if(LUA_SETTING_VAR && v3 != NULL) {
		objh->objp->phys_info.max_vel = *v3;
	}

	return lua_set_args(L, "o", l_Vector.Set(objh->objp->phys_info.max_vel));
}

LUA_VAR(Orientation, l_Object, "World orientation", "Object world orientation")
{
	object_h *objh;
	matrix_h *mh=NULL;
	if(!lua_get_args(L, "o|o", l_Object.GetPtr(&objh), l_Matrix.GetPtr(&mh)))
		return LUA_RETURN_NIL;

	if(!objh->IsValid())
		return LUA_RETURN_NIL;

	if(LUA_SETTING_VAR && mh != NULL) {
		if(mh->status == MH_MATRIX_OUTOFDATE) {
			vm_angles_2_matrix(&mh->mtx, &mh->ang);
		}
		objh->objp->orient = mh->mtx;
	}

	return lua_set_args(L, "o", l_Matrix.Set(matrix_h(&objh->objp->orient)));
}

LUA_VAR(HitpointsLeft, l_Object, "Number", "Hitpoints an object has left")
{
	object_h *objh = NULL;
	float f = -1.0f;
	if(!lua_get_args(L, "o|f", l_Object.GetPtr(&objh), &f))
		return LUA_RETURN_NIL;

	if(!objh->IsValid())
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
	if(LUA_SETTING_VAR && sobjh != NULL && sobjh->IsValid())
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

//**********HANDLE: Weapon
lua_obj<object_h> l_Weapon("Weapon", "Weapon handle", &l_Object);

LUA_VAR(Class, l_Weapon, "weaponclass", "Weapon's class")
{
	object_h *oh=NULL;
	int nc=-1;
	if(!lua_get_args(L, "o|o", l_Weapon.GetPtr(&oh), l_Weaponclass.Get(&nc)))
		return LUA_RETURN_NIL;

	if(!oh->IsValid())
		return LUA_RETURN_NIL;

	weapon *wp = &Weapons[oh->objp->instance];

	if(LUA_SETTING_VAR && nc > -1) {
		wp->weapon_info_index = nc;
	}

	return lua_set_args(L, "o", l_Weaponclass.Set(wp->weapon_info_index));
}

LUA_VAR(DestroyedByWeapon, l_Weapon, "boolean", "Whether weapon was destroyed by another weapon")
{
	object_h *oh=NULL;
	bool b = false;

	int numargs = lua_get_args(L, "o|b", l_Weapon.GetPtr(&oh), &b);
	
	if(!numargs)
		return LUA_RETURN_NIL;

	if(!oh->IsValid())
		return LUA_RETURN_NIL;

	weapon *wp = &Weapons[oh->objp->instance];

	if(LUA_SETTING_VAR && numargs > 1) {
		if(b)
			wp->weapon_flags |= WF_DESTROYED_BY_WEAPON;
		else
			wp->weapon_flags &= ~WF_DESTROYED_BY_WEAPON;
	}

	return lua_set_args(L, "b", (wp->weapon_flags & WF_DESTROYED_BY_WEAPON) > 0);
}

LUA_VAR(LifeLeft, l_Weapon, "number", "Weapon life left (in seconds)")
{
	object_h *oh=NULL;
	float nll = -1.0f;
	if(!lua_get_args(L, "o|f", l_Weapon.GetPtr(&oh), &nll))
		return LUA_RETURN_NIL;

	if(!oh->IsValid())
		return LUA_RETURN_NIL;

	weapon *wp = &Weapons[oh->objp->instance];

	if(LUA_SETTING_VAR && nll > -1.0f) {
		wp->lifeleft = nll;
	}

	return lua_set_args(L, "f", wp->lifeleft);
}

LUA_VAR(Team, l_Weapon, "team", "Weapon's team")
{
	object_h *oh=NULL;
	int nt=-1;
	if(!lua_get_args(L, "o|o", l_Weapon.GetPtr(&oh), l_Team.Get(&nt)))
		return LUA_RETURN_NIL;

	if(!oh->IsValid())
		return LUA_RETURN_NIL;

	weapon *wp = &Weapons[oh->objp->instance];

	if(LUA_SETTING_VAR && nt > -1) {
		wp->team = nt;
	}

	return lua_set_args(L, "o", l_Team.Set(wp->team));
}

//**********HANDLE: Camera
lua_obj<int> l_Camera("camera", "Camera");

LUA_FUNC(setPosition, l_Camera, "[world vector Position, number Translation Time, number Acceleration Time]", "True",
		"Sets camera position and velocity data."
		"Position is the final position for the camera. If not specified, the camera will simply stop at its current position."
		"Translation time is how long total, including acceleration, the camera should take to move. If it is not specified, the camera will jump to the specified position."
		"Acceleration time is how long it should take the camera to get 'up to speed'. If not specified, the camera will instantly start moving.")
{
	int idx=-1;
	vec3d *pos=NULL;
	float time=0.0f;
	float acc_time=0.0f;
	if(!lua_get_args(L, "o|off", l_Camera.Get(&idx), l_Vector.GetPtr(&pos), &time, &acc_time))
		return LUA_RETURN_NIL;
	
	if(idx < 0 || (uint)idx > Cameras.size())
		return LUA_RETURN_NIL;

	Cameras[idx].set_position(pos, time, acc_time);

	return LUA_RETURN_TRUE;
}

LUA_FUNC(setOrientation, l_Camera, "[world orientation Orientation, number Rotation Time, number Acceleration Time]", "True",
		"Sets camera orientation and velocity data."
		"<br>Orientation is the final orientation for the camera, after it has finished moving. If not specified, the camera will simply stop at its current orientation."
		"<br>Rotation time is how long total, including acceleration, the camera should take to rotate. If it is not specified, the camera will jump to the specified orientation."
		"<br>Acceleration time is how long it should take the camera to get 'up to speed'. If not specified, the camera will instantly start moving.")
{
	int idx=-1;
	matrix_h *mh=NULL;
	float time=0.0f;
	float acc_time=0.0f;
	if(!lua_get_args(L, "o|off", l_Camera.Get(&idx), l_Matrix.GetPtr(&mh), &time, &acc_time))
		return LUA_RETURN_NIL;
	
	if(idx < 0 || (uint)idx > Cameras.size())
		return LUA_RETURN_NIL;

	if(mh != NULL)
	{
		mh->ValidateMatrix();
		Cameras[idx].set_rotation(&mh->mtx, time, acc_time);
	}
	else
	{
		Cameras[idx].set_rotation();
	}

	return LUA_RETURN_TRUE;
}

//**********HANDLE: Weaponbank
#define SWH_NONE		0
#define SWH_PRIMARY		1
#define SWH_SECONDARY	2
#define SWH_TERTIARY	3

struct ship_banktype_h : public object_h
{
	int type;
	ship_weapon *sw;

	ship_banktype_h(object *objp, ship_weapon *wpn, int in_type) : object_h(objp) {
		sw = wpn;
		type = in_type;
	}
};
struct ship_bank_h : public ship_banktype_h
{
	int bank;

	ship_bank_h(object *objp, ship_weapon *wpn, int in_type, int in_bank) : ship_banktype_h(objp, wpn, in_type) {
		bank = in_bank;
	}
};

//**********HANDLE: Ship bank
lua_obj<ship_bank_h> l_WeaponBank("weaponbank", "Ship/subystem weapons bank handle");

LUA_VAR(Weapon, l_WeaponBank, "weaponclass", "Weapon")
{
	ship_bank_h *bh;
	int weaponclass=-1;
	if(!lua_get_args(L, "o|i", l_WeaponBank.GetPtr(&bh), l_Weaponclass.Get(&weaponclass)))
		return LUA_RETURN_NIL;

	if(!bh->IsValid())
		return LUA_RETURN_NIL;

	switch(bh->type)
	{
		case SWH_PRIMARY:
			if(LUA_SETTING_VAR && weaponclass > -1) {
				bh->sw->primary_bank_weapons[bh->bank] = weaponclass;
			}

			return lua_set_args(L, "o", l_Weaponclass.Set(bh->sw->primary_bank_weapons[bh->bank]));
		case SWH_SECONDARY:
			if(LUA_SETTING_VAR && weaponclass > -1) {
				bh->sw->secondary_bank_weapons[bh->bank] = weaponclass;
			}

			return lua_set_args(L, "o", l_Weaponclass.Set(bh->sw->secondary_bank_weapons[bh->bank]));
		case SWH_TERTIARY:
			if(LUA_SETTING_VAR && weaponclass > -1) {
				//bh->sw->tertiary_bank_weapons[bh->bank] = weaponclass;
			}

			//return lua_set_args(L, "o", l_Weaponclass.Set(bh->sw->tertiary_bank_weapons[bh->bank]));
			return LUA_RETURN_FALSE;
	}

	return LUA_RETURN_NIL;
}

LUA_VAR(AmmoLeft, l_WeaponBank, "number", "AmmoLeft")
{
	ship_bank_h *bh;
	int ammo;
	if(!lua_get_args(L, "o|i", l_WeaponBank.GetPtr(&bh), &ammo))
		return LUA_RETURN_NIL;

	if(!bh->IsValid())
		return LUA_RETURN_NIL;

	switch(bh->type)
	{
		case SWH_PRIMARY:
			if(LUA_SETTING_VAR && ammo > -1) {
				bh->sw->primary_bank_ammo[bh->bank] = ammo;
			}

			return lua_set_args(L, "i", bh->sw->primary_bank_ammo[bh->bank]);
		case SWH_SECONDARY:
			if(LUA_SETTING_VAR && ammo > -1) {
				bh->sw->secondary_bank_ammo[bh->bank] = ammo;
			}

			return lua_set_args(L, "i", bh->sw->secondary_bank_ammo[bh->bank]);
		case SWH_TERTIARY:
			if(LUA_SETTING_VAR && ammo > -1) {
				bh->sw->tertiary_bank_ammo = ammo;
			}

			return lua_set_args(L, "i", bh->sw->tertiary_bank_ammo);
	}

	return LUA_RETURN_NIL;
}

LUA_VAR(AmmoMax, l_WeaponBank, "number", "AmmoMax")
{
	ship_bank_h *bh;
	int ammomax;
	if(!lua_get_args(L, "o|i", l_WeaponBank.GetPtr(&bh), &ammomax))
		return LUA_RETURN_NIL;

	if(!bh->IsValid())
		return LUA_RETURN_NIL;

	switch(bh->type)
	{
		case SWH_PRIMARY:
			if(LUA_SETTING_VAR && ammomax > -1) {
				bh->sw->primary_bank_start_ammo[bh->bank] = ammomax;
			}

			return lua_set_args(L, "i", bh->sw->primary_bank_start_ammo[bh->bank]);
		case SWH_SECONDARY:
			if(LUA_SETTING_VAR && ammomax > -1) {
				bh->sw->secondary_bank_start_ammo[bh->bank] = ammomax;
			}

			return lua_set_args(L, "i", bh->sw->secondary_bank_start_ammo[bh->bank]);
		case SWH_TERTIARY:
			if(LUA_SETTING_VAR && ammomax > -1) {
				bh->sw->tertiary_bank_ammo = ammomax;
			}

			return lua_set_args(L, "i", bh->sw->tertiary_bank_start_ammo);
	}

	return LUA_RETURN_NIL;
}

//**********HANDLE: Weaponbanktype
lua_obj<ship_banktype_h> l_WeaponBankType("weaponbanktype", "Ship/subsystem weapons bank type handle");

LUA_VAR(Linked, l_WeaponBankType, "boolean", "Whether bank is in linked or unlinked fire mode (Primary-only)")
{
	ship_banktype_h *bh;
	bool newlink = false;
	int numargs = lua_get_args(L, "o|b", l_WeaponBankType.GetPtr(&bh), &newlink);

	if(!numargs)
		return LUA_RETURN_NIL;

	if(!bh->IsValid())
		return LUA_RETURN_NIL;

	switch(bh->type)
	{
		case SWH_PRIMARY:
			if(LUA_SETTING_VAR && numargs > 1) {
				if(newlink)
					Ships[bh->objp->instance].flags |= SF_PRIMARY_LINKED;
				else
					Ships[bh->objp->instance].flags &= ~SF_PRIMARY_LINKED;
			}

			return lua_set_args(L, "b", (Ships[bh->objp->instance].flags & SF_PRIMARY_LINKED) > 0);

		case SWH_SECONDARY:
		case SWH_TERTIARY:
			return LUA_RETURN_FALSE;
	}

	return LUA_RETURN_NIL;
}

LUA_VAR(DualFire, l_WeaponBankType, "boolean", "Whether bank is in dual fire mode (Secondary-only)")
{
	ship_banktype_h *bh;
	bool newfire = false;
	int numargs = lua_get_args(L, "o|b", l_WeaponBankType.GetPtr(&bh), &newfire);

	if(!numargs)
		return LUA_RETURN_NIL;

	if(!bh->IsValid())
		return LUA_RETURN_NIL;

	switch(bh->type)
	{
		case SWH_SECONDARY:
			if(LUA_SETTING_VAR && numargs > 1) {
				if(newfire)
					Ships[bh->objp->instance].flags |= SF_SECONDARY_DUAL_FIRE;
				else
					Ships[bh->objp->instance].flags &= ~SF_SECONDARY_DUAL_FIRE;
			}

			return lua_set_args(L, "b", (Ships[bh->objp->instance].flags & SF_SECONDARY_DUAL_FIRE) > 0);

		case SWH_PRIMARY:
		case SWH_TERTIARY:
			return LUA_RETURN_FALSE;
	}

	return LUA_RETURN_NIL;
}

LUA_INDEXER(l_WeaponBankType, "Bank index", "weaponbank handle", "Returns handle to a specific weapon bank")
{
	ship_banktype_h *sb=NULL;
	int idx = -1;
	ship_bank_h *newbank;
	if(!lua_get_args(L, "oi|o", l_WeaponBankType.GetPtr(&sb), &idx, l_WeaponBank.GetPtr(&newbank)))
		return LUA_RETURN_NIL;

	if(!sb->IsValid())
		return LUA_RETURN_NIL;

	switch(sb->type)
	{
		case SWH_PRIMARY:
				if(idx < 1 || idx > sb->sw->num_primary_banks)
					return LUA_RETURN_FALSE;

				idx--; //Lua->FS2

				if(LUA_SETTING_VAR && newbank->IsValid()) {
					//WMC: TODO
				}
				break;
		case SWH_SECONDARY:
				if(idx < 1 || idx > sb->sw->num_secondary_banks)
					return LUA_RETURN_FALSE;

				idx--; //Lua->FS2

				if(LUA_SETTING_VAR && newbank->IsValid()) {
					//WMC: TODO
				}
				break;
		case SWH_TERTIARY:
				if(idx < 1 || idx > sb->sw->num_tertiary_banks)
					return LUA_RETURN_FALSE;

				idx--; //Lua->FS2

				if(LUA_SETTING_VAR && newbank->IsValid()) {
					//WMC: TODO
				}
				break;
		default:
			return LUA_RETURN_NIL;	//Invalid type
	}

	return lua_set_args(L, "o", l_WeaponBank.Set(ship_bank_h(sb->objp, sb->sw, sb->type, idx)));
}

LUA_FUNC(getNum, l_WeaponBank, NULL, "Number of weapons mounted in bank", "Gets the number of weapons in the mounted bank")
{
	ship_banktype_h *sb=NULL;
	if(!lua_get_args(L, "o", l_WeaponBankType.GetPtr(&sb)))
		return LUA_RETURN_NIL;

	if(!sb->IsValid())
		return LUA_RETURN_NIL;

	switch(sb->type)
	{
		case SWH_PRIMARY:
			return lua_set_args(L, "i", sb->sw->num_primary_banks);
		case SWH_SECONDARY:
			return lua_set_args(L, "i", sb->sw->num_secondary_banks);
		case SWH_TERTIARY:
			return lua_set_args(L, "i", sb->sw->num_tertiary_banks);
		default:
			return LUA_RETURN_NIL;	//Invalid type
	}
}

//**********HANDLE: Subsystem
struct ship_subsys_h : public object_h
{
	ship_subsys *ss;	//Pointer to subsystem, or NULL for the hull

	bool IsValid(){return objp->signature == sig;}
	ship_subsys_h(object *objp, ship_subsys *sub) : object_h(objp) {
		ss = sub;
	}
};
lua_obj<ship_subsys_h> l_Subsystem("subsystem", "Ship subsystem handle");

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

LUA_VAR(Position, l_Subsystem, "local vector", "Subsystem position with regards to main ship")
{
	ship_subsys_h *sso;
	vec3d *v = NULL;
	if(!lua_get_args(L, "o|o", l_Subsystem.GetPtr(&sso), l_Vector.GetPtr(&v)))
		return LUA_RETURN_NIL;

	if(!sso->IsValid())
		return LUA_RETURN_NIL;

	polymodel *pm = model_get(Ships[sso->objp->instance].modelnum);
	Assert(pm != NULL);

	bsp_info *sm = &pm->submodel[sso->ss->system_info->subobj_num];

	if(LUA_SETTING_VAR && v != NULL)
		sm->offset = *v;

	return lua_set_args(L, "o", l_Vector.Set(sm->offset));
}

LUA_VAR(GunPosition, l_Subsystem, "local vector", "Subsystem gun position with regards to main ship")
{
	ship_subsys_h *sso;
	vec3d *v = NULL;
	if(!lua_get_args(L, "o|o", l_Subsystem.GetPtr(&sso), l_Vector.GetPtr(&v)))
		return LUA_RETURN_NIL;

	if(!sso->IsValid())
		return LUA_RETURN_NIL;

	polymodel *pm = model_get(Ships[sso->objp->instance].modelnum);
	Assert(pm != NULL);

	if(sso->ss->system_info->turret_gun_sobj < 0)
		return LUA_RETURN_NIL;

	bsp_info *sm = &pm->submodel[sso->ss->system_info->turret_gun_sobj];

	if(LUA_SETTING_VAR && v != NULL)
		sm->offset = *v;

	return lua_set_args(L, "o", l_Vector.Set(sm->offset));
}


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

LUA_FUNC(getName, l_Subsystem, NULL, "Subsystem name", "Subsystem name")
{
	ship_subsys_h *sso;
	char *s = NULL;
	if(!lua_get_args(L, "o|f", l_Subsystem.GetPtr(&sso), &s))
		return LUA_RETURN_NIL;

	if(!sso->IsValid())
		return LUA_RETURN_NIL;

	return lua_set_args(L, "s", sso->ss->system_info->name);
}

//**********HANDLE: Texture
lua_obj<int> l_Texture("texture", "Texture handle");
//WMC - int should NEVER EVER be an invalid handle. Return Nil instead. Nil FTW.

static float lua_Opacity = 1.0f;
static int lua_Opacity_type = GR_ALPHABLEND_NONE;

LUA_INDEXER(l_Texture, "Index", "Texture handle",
			"Returns texture handle to specified frame number in current texture's animation."
			"This means that [1] will always return the first frame in an animation, no matter what frame an animation is."
			"You cannot change a texture animation frame.")
{
	int idx;
	int frame=-1;
	int newframe=-1;	//WMC - Ignore for now
	if(!lua_get_args(L, "oi|i", l_Texture.Get(&idx), &frame, &newframe))
		return LUA_RETURN_NIL;

	if(frame < 1)
		return LUA_RETURN_NIL;

	//Get me some info
	int num=-1;
	int first=-1;
	first = bm_get_info(idx, NULL, NULL, NULL, &num);

	//Check it's a valid one
	if(first < 0 || frame > num)
		return LUA_RETURN_NIL;

	frame--; //Lua->FS2

	//Get actual texture handle
	frame = first + frame;

	return lua_set_args(L, "o", l_Texture.Set(frame));
}

LUA_FUNC(unload, l_Texture, NULL, NULL, "Unloads a texture from memory")
{
	int idx;

	if(!lua_get_args(L, "o", l_Texture.Get(&idx)))
		return LUA_RETURN_NIL;

	bm_unload(idx);

	return LUA_RETURN_NIL;
}

LUA_FUNC(getWidth, l_Texture, NULL, "Texture width, or false if invalid handle", "Gets texture width")
{
	int idx;
	if(!lua_get_args(L, "o", l_Texture.Get(&idx)))
		return LUA_RETURN_NIL;

	int w = -1;

	if(bm_get_info(idx, &w) < 0)
		return LUA_RETURN_FALSE;
	else
		return lua_set_args(L, "i", w);
}

LUA_FUNC(getHeight, l_Texture, NULL, "Texture height, or false if invalid handle", "Gets texture width")
{
	int idx;
	if(!lua_get_args(L, "o", l_Texture.Get(&idx)))
		return LUA_RETURN_NIL;

	int h=-1;

	if(bm_get_info(idx, NULL, &h) < 0)
		return LUA_RETURN_FALSE;
	else
		return lua_set_args(L, "i", h);
}

LUA_FUNC(getFPS, l_Texture, NULL, "Texture FPS", "Gets frames-per-second of texture")
{
	int idx;
	if(!lua_get_args(L, "o", l_Texture.Get(&idx)))
		return LUA_RETURN_NIL;

	int fps=-1;

	if(bm_get_info(idx, NULL, NULL, NULL, NULL, &fps) < 0)
		return LUA_RETURN_FALSE;
	else
		return lua_set_args(L, "i", fps);
}

LUA_FUNC(getFramesLeft, l_Texture, NULL, "Number of frames left", "Gets number of frames left, from handle's position in animation")
{
	int idx;
	if(!lua_get_args(L, "o", l_Texture.Get(&idx)))
		return LUA_RETURN_NIL;

	int num=-1;

	if(bm_get_info(idx, NULL, NULL, NULL, &num) < 0)
		return LUA_RETURN_FALSE;
	else
		return lua_set_args(L, "i", num);
}


//**********HANDLE: Textures
struct ship_textures_h : public object_h
{
	ship_textures_h(object *objp) : object_h(objp){}
};

lua_obj<ship_textures_h> l_ShipTextures("shiptextures", "Ship textures handle");

LUA_INDEXER(l_ShipTextures, "Texture name or index", "Texture", "Ship textures")
{
	ship_textures_h *sh;
	char *s;
	int tdx=-1;
	if(!lua_get_args(L, "os|o", l_ShipTextures.GetPtr(&sh), &s, l_Texture.Get(&tdx)))
		return LUA_RETURN_NIL;

	if(!sh->IsValid() || s==NULL)
		return LUA_RETURN_NIL;

	ship *shipp = &Ships[sh->objp->instance];
	polymodel *pm = model_get(shipp->modelnum);
	int idx = -1;
	int i;

	char fname[MAX_FILENAME_LEN];
	char *p;
	for(i = 0; i < pm->n_textures; i++)
	{
		if(pm->textures[i] > -1)
		{
			bm_get_filename(pm->textures[i], fname);

			//Get rid of extension
			p = strchr( fname, '.' );
			if ( p != NULL)
				*p = 0;

			if(!stricmp(fname, s)) {
				idx = i;
				break;
			}
		}

		if(shipp->replacement_textures != NULL && pm->textures[i] > -1)
		{
			bm_get_filename(shipp->replacement_textures[i], fname);
			//Get rid of extension
			p = strchr( fname, '.' );
			if ( p != NULL)
				*p = 0;

			if(!stricmp(fname, s)) {
				idx = i;
				break;
			}
		}
	}

	if(idx < 0)
	{
		i = atoi(s);
		if(i < 1 || i > pm->n_textures)
			return LUA_RETURN_FALSE;

		i--; //Lua->FS2
		idx = i;
	}

	if(LUA_SETTING_VAR && tdx > -1) {
		shipp->replacement_textures = shipp->replacement_textures_buf;
		shipp->replacement_textures[idx] = tdx;
	}

	if(shipp->replacement_textures != NULL && shipp->replacement_textures[idx] > -1)
		return lua_set_args(L, "o", l_Texture.Set(shipp->replacement_textures[idx]));
	else if(pm->textures[idx] > -1)
		return lua_set_args(L, "o", l_Texture.Set(pm->textures[idx]));
	else
		return LUA_RETURN_FALSE;
}

//**********HANDLE: Ship
lua_obj<object_h> l_Ship("ship", "Ship handle", &l_Object);

LUA_INDEXER(l_Ship, "Subsystem name or index", "Subsystem", "Returns subsystem based on name or index passed")
{
	object_h *objh;
	char *s = NULL;
	ship_subsys_h *sub;
	if(!lua_get_args(L, "o|so", l_Ship.GetPtr(&objh), &s, l_Subsystem.GetPtr(&sub)))
		return LUA_RETURN_NIL;

	if(!objh->IsValid())
		return LUA_RETURN_NIL;

	ship *shipp = &Ships[objh->objp->instance];
	ship_subsys *ss = ship_get_subsys(shipp, s);

	if(ss == NULL)
	{
		int idx = atoi(s);
		if(idx > 0 && idx <= shipp->n_subsystems)
		{
			ss = ship_get_indexed_subsys(shipp, idx);
		}
	}

	if(ss == NULL)
		return LUA_RETURN_NIL;

	return lua_set_args(L, "o", l_Subsystem.Set(ship_subsys_h(objh->objp, ss)));
}

LUA_VAR(Name, l_Ship, "String", "Ship name")
{
	object_h *objh;
	char *s = NULL;
	if(!lua_get_args(L, "o|s", l_Ship.GetPtr(&objh), &s))
		return LUA_RETURN_NIL;

	if(!objh->IsValid())
		return LUA_RETURN_NIL;

	ship *shipp = &Ships[objh->objp->instance];

	if(LUA_SETTING_VAR && s != NULL) {
		strncpy(shipp->ship_name, s, sizeof(shipp->ship_name)-1);
	}

	return lua_set_args(L, "s", shipp->ship_name);
}

LUA_VAR(AfterburnerFuelLeft, l_Ship, "Number", "Afterburner fuel left")
{
	object_h *objh;
	float fuel = -1.0f;
	if(!lua_get_args(L, "o|f", l_Ship.GetPtr(&objh), &fuel))
		return LUA_RETURN_NIL;

	if(!objh->IsValid())
		return LUA_RETURN_NIL;

	ship *shipp = &Ships[objh->objp->instance];

	if(LUA_SETTING_VAR && fuel >= 0.0f)
		shipp->afterburner_fuel = fuel;

	return lua_set_args(L, "f", shipp->afterburner_fuel);
}

LUA_VAR(AfterburnerFuelMax, l_Ship, "Number", "Afterburner fuel capacity")
{
	object_h *objh;
	float fuel = -1.0f;
	if(!lua_get_args(L, "o|f", l_Ship.GetPtr(&objh), &fuel))
		return LUA_RETURN_NIL;

	if(!objh->IsValid())
		return LUA_RETURN_NIL;

	ship_info *sip = &Ship_info[Ships[objh->objp->instance].ship_info_index];

	if(LUA_SETTING_VAR && fuel >= 0.0f)
		sip->afterburner_fuel_capacity = fuel;

	return lua_set_args(L, "f", sip->afterburner_fuel_capacity);
}

LUA_VAR(Class, l_Ship, "shipclass", "Ship class")
{
	object_h *objh;
	int idx=-1;
	if(!lua_get_args(L, "o|o", l_Ship.GetPtr(&objh), l_Shipclass.Get(&idx)))
		return LUA_RETURN_NIL;

	if(!objh->IsValid())
		return LUA_RETURN_NIL;

	ship *shipp = &Ships[objh->objp->instance];

	if(LUA_SETTING_VAR && idx > -1)
		shipp->ship_info_index = idx;

	if(shipp->ship_info_index < 0)
		return LUA_RETURN_NIL;

	return lua_set_args(L, "o", l_Shipclass.Set(shipp->ship_info_index));
}

LUA_VAR(CountermeasuresLeft, l_Ship, "Number", "Number of countermeasures left")
{
	object_h *objh;
	int newcm = -1;
	if(!lua_get_args(L, "o|i", l_Ship.GetPtr(&objh), &newcm))
		return LUA_RETURN_NIL;

	if(!objh->IsValid())
		return LUA_RETURN_NIL;

	ship *shipp = &Ships[objh->objp->instance];

	if(LUA_SETTING_VAR && newcm > -1)
		shipp->cmeasure_count = newcm;

	return lua_set_args(L, "i", shipp->cmeasure_count);
}

LUA_VAR(CountermeasureClass, l_Ship, "Number", "Weapon class mounted on this ship's countermeasure point")
{
	object_h *objh;
	int newcm = -1;
	if(!lua_get_args(L, "o|o", l_Ship.GetPtr(&objh), l_Weaponclass.Get(&newcm)))
		return LUA_RETURN_NIL;

	if(!objh->IsValid())
		return LUA_RETURN_NIL;

	ship *shipp = &Ships[objh->objp->instance];

	if(LUA_SETTING_VAR) {
			shipp->current_cmeasure = newcm;
	}

	if(shipp->current_cmeasure > -1)
		return lua_set_args(L, "o", l_Weaponclass.Set(shipp->current_cmeasure));
	else
		return LUA_RETURN_NIL;
}

LUA_VAR(HitpointsMax, l_Ship, "Number", "Total hitpoints")
{
	object_h *objh;
	float newhits = -1;
	if(!lua_get_args(L, "o|f", l_Ship.GetPtr(&objh), &newhits))
		return LUA_RETURN_NIL;

	if(!objh->IsValid())
		return LUA_RETURN_NIL;

	ship *shipp = &Ships[objh->objp->instance];

	if(LUA_SETTING_VAR && newhits > -1)
		shipp->ship_max_hull_strength = newhits;

	return lua_set_args(L, "f", shipp->ship_max_hull_strength);
}

LUA_VAR(PrimaryBanks, l_Ship, "weaponbanktype", "Array of primary weapon banks")
{
	object_h *objh;
	ship_banktype_h *swh;
	if(!lua_get_args(L, "o|o", l_Ship.GetPtr(&objh), l_WeaponBankType.GetPtr(&swh)))
		return LUA_RETURN_NIL;

	if(!objh->IsValid())
		return LUA_RETURN_NIL;

	ship_weapon *dst = &Ships[objh->objp->instance].weapons;

	if(LUA_SETTING_VAR && swh->IsValid()) {
		ship_weapon *src = &Ships[swh->objp->instance].weapons;

		dst->current_primary_bank = src->current_primary_bank;
		dst->num_primary_banks = src->num_primary_banks;

		memcpy(dst->next_primary_fire_stamp, src->next_primary_fire_stamp, sizeof(dst->next_primary_fire_stamp));
		memcpy(dst->primary_animation_done_time, src->primary_animation_done_time, sizeof(dst->primary_animation_done_time));
		memcpy(dst->primary_animation_position, src->primary_animation_position, sizeof(dst->primary_animation_position));
		memcpy(dst->primary_bank_ammo, src->primary_bank_ammo, sizeof(dst->primary_bank_ammo));
		memcpy(dst->primary_bank_capacity, src->primary_bank_capacity, sizeof(dst->primary_bank_capacity));
		memcpy(dst->primary_bank_rearm_time, src->primary_bank_rearm_time, sizeof(dst->primary_bank_rearm_time));
		memcpy(dst->primary_bank_start_ammo, src->primary_bank_start_ammo, sizeof(dst->primary_bank_start_ammo));
		memcpy(dst->primary_bank_weapons, src->primary_bank_weapons, sizeof(dst->primary_bank_weapons));
		memcpy(dst->primary_next_slot, src->primary_next_slot, sizeof(dst->primary_next_slot));
	}

	return lua_set_args(L, "o", l_WeaponBankType.Set(ship_banktype_h(objh->objp, dst, SWH_PRIMARY)));
}

LUA_VAR(SecondaryBanks, l_Ship, "weaponbanktype", "Array of secondary weapon banks")
{
	object_h *objh;
	ship_banktype_h *swh;
	if(!lua_get_args(L, "o|o", l_Ship.GetPtr(&objh), l_WeaponBankType.GetPtr(&swh)))
		return LUA_RETURN_NIL;

	if(!objh->IsValid())
		return LUA_RETURN_NIL;

	ship_weapon *dst = &Ships[objh->objp->instance].weapons;

	if(LUA_SETTING_VAR && swh->IsValid()) {
		ship_weapon *src = &Ships[swh->objp->instance].weapons;

		dst->current_secondary_bank = src->current_secondary_bank;
		dst->num_secondary_banks = src->num_secondary_banks;

		memcpy(dst->next_secondary_fire_stamp, src->next_secondary_fire_stamp, sizeof(dst->next_secondary_fire_stamp));
		memcpy(dst->secondary_animation_done_time, src->secondary_animation_done_time, sizeof(dst->secondary_animation_done_time));
		memcpy(dst->secondary_animation_position, src->secondary_animation_position, sizeof(dst->secondary_animation_position));
		memcpy(dst->secondary_bank_ammo, src->secondary_bank_ammo, sizeof(dst->secondary_bank_ammo));
		memcpy(dst->secondary_bank_capacity, src->secondary_bank_capacity, sizeof(dst->secondary_bank_capacity));
		memcpy(dst->secondary_bank_rearm_time, src->secondary_bank_rearm_time, sizeof(dst->secondary_bank_rearm_time));
		memcpy(dst->secondary_bank_start_ammo, src->secondary_bank_start_ammo, sizeof(dst->secondary_bank_start_ammo));
		memcpy(dst->secondary_bank_weapons, src->secondary_bank_weapons, sizeof(dst->secondary_bank_weapons));
		memcpy(dst->secondary_next_slot, src->secondary_next_slot, sizeof(dst->secondary_next_slot));
	}

	return lua_set_args(L, "o", l_WeaponBankType.Set(ship_banktype_h(objh->objp, dst, SWH_SECONDARY)));
}

LUA_VAR(TertiaryBanks, l_Ship, "weaponbanktype", "Array of tertiary weapon banks")
{
	object_h *objh;
	ship_banktype_h *swh;
	if(!lua_get_args(L, "o|o", l_Ship.GetPtr(&objh), l_WeaponBankType.GetPtr(&swh)))
		return LUA_RETURN_NIL;

	if(!objh->IsValid())
		return LUA_RETURN_NIL;

	ship_weapon *dst = &Ships[objh->objp->instance].weapons;

	if(LUA_SETTING_VAR && swh->IsValid()) {
		ship_weapon *src = &Ships[swh->objp->instance].weapons;

		dst->current_tertiary_bank = src->current_tertiary_bank;
		dst->num_tertiary_banks = src->num_tertiary_banks;

		dst->next_tertiary_fire_stamp = src->next_tertiary_fire_stamp;
		dst->tertiary_bank_ammo = src->tertiary_bank_ammo;
		dst->tertiary_bank_capacity = src->tertiary_bank_capacity;
		dst->tertiary_bank_rearm_time = src->tertiary_bank_rearm_time;
		dst->tertiary_bank_start_ammo = src->tertiary_bank_start_ammo;
	}

	return lua_set_args(L, "o", l_WeaponBankType.Set(ship_banktype_h(objh->objp, dst, SWH_TERTIARY)));
}

LUA_VAR(Target, l_Ship, "object", "Target of ship. Value may also be a deriviative of the 'object' class, such as 'ship'.")
{
	object_h *objh;
	object_h *newh;
	//WMC - Maybe use two argument return capabilities of Lua to set/return subsystem?
	if(!lua_get_args(L, "o|o", l_Ship.GetPtr(&objh), l_Object.GetPtr(&newh)))
		return LUA_RETURN_NIL;

	if(!objh->IsValid())
		return LUA_RETURN_NIL;

	ai_info *aip = NULL;
	if(Ships[objh->objp->instance].ai_index > -1)
		aip = &Ai_info[Ships[objh->objp->instance].ai_index];
	else
		return LUA_RETURN_FALSE;

	if(LUA_SETTING_VAR && newh != NULL && newh->IsValid())
	{
		if(aip->target_signature != newh->sig)
		{
			aip->target_objnum = OBJ_INDEX(newh->objp);
			aip->target_signature = newh->sig;
			aip->target_time = 0.0f;
			set_targeted_subsys(aip, NULL, -1);
		}
	}

	switch(Objects[aip->target_objnum].type)
	{
		case OBJ_SHIP:
			return lua_set_args(L, "o", l_Ship.Set(object_h(&Objects[aip->target_objnum])));
		default:
			return lua_set_args(L, "o", l_Object.Set(object_h(&Objects[aip->target_objnum])));
	}
}

LUA_VAR(Team, l_Ship, "team", "Ship's team")
{
	object_h *oh=NULL;
	int nt=-1;
	if(!lua_get_args(L, "o|o", l_Ship.GetPtr(&oh), l_Team.Get(&nt)))
		return LUA_RETURN_NIL;

	if(!oh->IsValid())
		return LUA_RETURN_NIL;

	ship *shipp = &Ships[oh->objp->instance];

	if(LUA_SETTING_VAR && nt > -1) {
		shipp->team = nt;
	}

	return lua_set_args(L, "o", l_Team.Set(shipp->team));
}

LUA_VAR(Textures, l_Ship, "shiptextures", "Gets ship textures")
{
	object_h *objh;
	ship_textures_h *th;
	if(!lua_get_args(L, "o|o", l_Ship.GetPtr(&objh), l_ShipTextures.GetPtr(&th)))
		return LUA_RETURN_NIL;

	if(!objh->IsValid())
		return LUA_RETURN_NIL;

	if(LUA_SETTING_VAR && th != NULL && th->IsValid()) {
		ship *src = &Ships[objh->objp->instance];
		ship *dest = &Ships[th->objp->instance];
		if(src->replacement_textures != NULL)
		{
			dest->replacement_textures = dest->replacement_textures_buf;
			memcpy(dest->replacement_textures_buf, src->replacement_textures_buf, sizeof(dest->replacement_textures_buf));
		}
	}

	return lua_set_args(L, "o", l_ShipTextures.Set(ship_textures_h(objh->objp)));
}

LUA_FUNC(getNumSubsystems, l_Ship, NULL, "Number of subsystems", "Gets number of subsystems on ship")
{
	object_h *objh;
	if(!lua_get_args(L, "o", l_Ship.GetPtr(&objh)))
		return LUA_RETURN_NIL;

	if(!objh->IsValid())
		return LUA_RETURN_NIL;

	return lua_set_args(L, "i", Ships[objh->objp->instance].n_subsystems);
}

LUA_FUNC(getAnimationDoneTime, l_Ship, "Type, Subtype", "Time (milliseconds)", "Gets time that animation will be done")
{
	object_h *objh;
	char *s = NULL;
	int subtype=-1;
	if(!lua_get_args(L, "o|si", l_Ship.GetPtr(&objh), &s, &subtype))
		return LUA_RETURN_NIL;

	if(!objh->IsValid())
		return LUA_RETURN_NIL;

	int type = match_animation_type(s);
	if(type < 0)
		return LUA_RETURN_FALSE;

	return lua_set_args(L, "i", ship_get_animation_time_type(&Ships[objh->objp->instance], type, subtype));
}

LUA_FUNC(triggerAnimation, l_Ship, "Type, [Subtype, Forwards]", "True",
		 "Triggers an animation. Type is the string name of the animation type, "
		 "Subtype is the subtype number, such as weapon bank #, and Forwards is boolean."
		 "<br><strong>IMPORTANT: Function is in testing and should not be used with official mod releases</strong>")
{
	object_h *objh;
	char *s = NULL;
	bool b = true;
	int subtype=-1;
	if(!lua_get_args(L, "o|sib", l_Ship.GetPtr(&objh), &s, &subtype, &b))
		return LUA_RETURN_NIL;

	if(!objh->IsValid())
		return LUA_RETURN_NIL;

	int type = match_animation_type(s);
	if(type < 0)
		return LUA_RETURN_FALSE;

	int dir = 1;
	if(!b)
		dir = -1;

	ship_start_animation_type(&Ships[objh->objp->instance], type, subtype, dir);

	return LUA_RETURN_TRUE;
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

//**********HANDLE: Wing
lua_obj<int> l_Wing("wing", "Wing handle");

LUA_INDEXER(l_Wing, "Index", "Ship", "Ship via number in wing")
{
	int wdx;
	int sdx;
	object_h *ndx=NULL;
	if(!lua_get_args(L, "oi|o", l_Wing.Get(&wdx), &sdx, l_Ship.GetPtr(&ndx)))
		return LUA_RETURN_NIL;

	if(sdx < Wings[wdx].current_count) {
		return LUA_RETURN_NIL;
	}

	if(LUA_SETTING_VAR && ndx != NULL && ndx->IsValid()) {
		Wings[wdx].ship_index[sdx] = ndx->objp->instance;
	}

	return lua_set_args(L, "o", l_Ship.Set(object_h(&Objects[Ships[Wings[wdx].ship_index[sdx]].objnum])));
}
//**********HANDLE: Player
lua_obj<int> l_Player("player", "Player handle");

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

LUA_FUNC(getCampaignName, l_Player, NULL, "Campaign name (string)", "Gets current player campaign")
{
	int idx;
	player_helper(L, &idx);

	return lua_set_args(L, "s", Players[idx].current_campaign);
}

LUA_FUNC(getImage, l_Player, NULL, "Player image (string)", "Gets current player image")
{
	int idx;
	player_helper(L, &idx);

	return lua_set_args(L, "s", Players[idx].image_filename);
}


LUA_FUNC(getMainHall, l_Player, NULL, "Main hall number", "Gets player's main hall number")
{
	int idx;
	player_helper(L, &idx);

	return lua_set_args(L, "i", (int)Players[idx].main_hall);
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
//**********LIBRARY: Base
lua_lib l_Base("Base", "ba", "Base Freespace 2 functions");

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

LUA_FUNC(getStateNameByIndex, l_Base, "Index of state (number)", "State name (string)", "Gets the name of a state type by its index; this function may be used to list all state types.")
{
	int i;
	if(!lua_get_args(L, "i", &i))
		return LUA_RETURN_NIL;

	//Lua->FS2
	i--;

	if(i < 0 || i >= Num_gs_state_text)
		return LUA_RETURN_NIL;

	return lua_set_args(L, "s", GS_state_text[i]);
}

LUA_FUNC(getNumStates, l_Base, NULL, "Number of states", "Gets the number of different state types currently implemented in FS2_Open")
{
	return lua_set_args(L, "i", Num_gs_state_text);
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

LUA_FUNC(getEventNameByIndex, l_Base, "Index of event type (number)", "Event name (string)", "Gets the name of a event type, given an index; this function may be used to list all event dealt with by setEvent()")
{
	int i;
	if(!lua_get_args(L, "i", &i))
		return LUA_RETURN_NIL;

	//Lua->FS2
	i--;

	if(i < 0 || i >= Num_gs_event_text)
		return LUA_RETURN_NIL;

	return lua_set_args(L, "s", GS_event_text[i]);
}

LUA_FUNC(getNumEvents, l_Base, NULL, "Number of event types", "Gets the number of different event types currently implemented in FS2")
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

LUA_FUNC(getNumPlayers, l_Base, NULL, "Number of players", "Gets the number of currently loaded players")
{
	return lua_set_args(L, "i", Player_num);
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


//**********LIBRARY: Math
lua_lib l_Math("Math", "ma", "Math library");

LUA_FUNC(getRandomNumber, l_Math, "[Smallest number, Largest number]", "Random number", "Returns a random number; default is 0 to 1. May be non-whole.")
{
	float min = 0.0f;
	float max = 1.0f;
	lua_get_args(L, "ff", &min, &max);

	if(max < min)
		return LUA_RETURN_NIL;

	return lua_set_args(L, "f", frand_range(min, max));
}

LUA_FUNC(newVector, l_Math, "[x, y, z]", "Vector object", "Creates a vector object")
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

LUA_FUNC(getMissionTime, l_Mission, NULL, "number", "Mission time in seconds")
{
	if(!(Game_mode & GM_IN_MISSION))
		return LUA_RETURN_NIL;

	/*
	if(LUA_SETTING_VAR)
	{
		fix newtime=Missiontime;
		lua_get_args(L, "|x", &newtime);
		Missiontime = newtime;
	}*/

	return lua_set_args(L, "x", Missiontime);
}

//WMC - These are in freespace.cpp
LUA_FUNC(loadMission, l_Mission, "Mission name", "True if mission was loaded, false otherwise", "Loads a mission")
{
	char *s;
	if(!lua_get_args(L, "s", &s))
		return LUA_RETURN_NIL;

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

LUA_FUNC(unloadMission, l_Mission, NULL, NULL, "Unloads a loaded mission")
{
	if(Game_mode & GM_IN_MISSION)
	{
		game_level_close();
		Game_mode &= ~GM_IN_MISSION;
		strcpy(Game_current_mission_filename, "");
	}

	return LUA_RETURN_NIL;
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

LUA_FUNC(getShipByName, l_Mission, "Ship name", "Ship object", "Gets ship object")
{
	char *name;
	if(!lua_get_args(L, "s", &name))
		return LUA_RETURN_NIL;

	int idx = ship_name_lookup(name);
	
	if(idx < 0) {
		return LUA_RETURN_NIL;
	}

	return lua_set_args(L, "o", l_Ship.Set(object_h(&Objects[Ships[idx].objnum])));
}

LUA_FUNC(getNumShips, l_Mission, NULL, "Number of ships in mission", "Gets number of ships in mission. Note that this is only accurate for a short while.")
{
	return lua_set_args(L, "i", ship_get_num_ships());
}

LUA_FUNC(getShipByIndex, l_Mission, "Index", "Ship handle, or false if invalid index",
		"Gets ship by its order in the mission."
		"Note that as ships are added, they may take the index of destroyed ships")
{
	int idx;
	if(!lua_get_args(L, "i", &idx))
		return LUA_RETURN_NIL;

	//Remember, Lua indices start at 0.
	int count=1;

	for(int i = 0; i < MAX_SHIPS; i++)
	{
		if (Ships[i].objnum < 0 || Objects[Ships[i].objnum].type != OBJ_SHIP)
			continue;

		if(count == idx) {
			return lua_set_args(L, "o", l_Ship.Set(object_h(&Objects[Ships[i].objnum])));
		}

		count++;
	}

	return LUA_RETURN_FALSE;
}

LUA_FUNC(getWing, l_Mission, "Wing name", "Wing handle", "Gets wing handle")
{
	char *name;
	if(!lua_get_args(L, "s", &name))
		return 0;

	int idx = wing_name_lookup(name);
	
	if(idx < 0) {
		return LUA_RETURN_NIL;
	}

	return lua_set_args(L, "o", l_Wing.Set(idx));
}

LUA_FUNC(getNumWings, l_Mission, NULL, "Number of ships in mission", "Gets number of ships in mission")
{
	return lua_set_args(L, "i", Num_wings);
}

LUA_FUNC(getWingByIndex, l_Mission, "Index", "Wing handle, or false if invalid index", "Gets wing by its index in the mission")
{
	int idx;
	if(!lua_get_args(L, "i", &idx))
		return LUA_RETURN_NIL;

	if(idx < 1 || idx > Num_wings)
		return LUA_RETURN_FALSE;
	
	idx--; //Lua->FS2

	return lua_set_args(L, "o", l_Wing.Set(idx));
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

LUA_FUNC(getWeaponClassByName, l_Tables, "Class name", "Weaponclass object", "Gets weapon class")
{
	if(!ships_inited)
		return LUA_RETURN_NIL;

	char *name;
	if(!lua_get_args(L, "s", &name))
		return 0;

	int idx = weapon_info_lookup(name);
	
	if(idx < 0) {
		return LUA_RETURN_NIL;
	}

	return lua_set_args(L, "o", l_Weaponclass.Set(idx));
}

LUA_FUNC(getNumWeaponClasses, l_Tables, NULL, "Number", "Gets number of weapon classes")
{
	if(!ships_inited)
		return lua_set_args(L, "i", 0);	//No ships loaded...should be 0

	return lua_set_args(L, "i", Num_weapon_types);
}

LUA_FUNC(getWeaponClassByIndex, l_Tables, "Class index", "Weaponclass handle", "Gets weanon class by index")
{
	if(!ships_inited)
		return LUA_RETURN_NIL;

	int idx;
	if(!lua_get_args(L, "i", &idx))
		return 0;

	//Lua->FS2
	idx--;
	
	if(idx < 0 || idx > Num_weapon_types) {
		return LUA_RETURN_NIL;
	}

	return lua_set_args(L, "o", l_Weaponclass.Set(idx));
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

LUA_FUNC(getX, l_Mouse, NULL, "X pos (Number)", "Gets Mouse X pos")
{
	if(!mouse_inited)
		return LUA_RETURN_NIL;

	int x;

	mouse_get_pos_unscaled(&x, NULL);

	return lua_set_args(L, "i", x);
}

LUA_FUNC(getY, l_Mouse, NULL, "Y pos (Number)", "Gets Mouse Y pos")
{
	if(!mouse_inited)
		return LUA_RETURN_NIL;

	int y;

	mouse_get_pos_unscaled(NULL, &y);

	return lua_set_args(L, "i", y);
}

LUA_FUNC(isButtonDown, l_Mouse, "{Left, Right, or Middle}, [..., ...]", "Whether specified buttons are pressed (Boolean)", "Returns whether the specified mouse buttons are up or down")
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

LUA_FUNC(clearScreen, l_Graphics, "[Red, green, blue]", NULL, "Clears the screen to black, or the color specified.")
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

LUA_FUNC(getFontHeight, l_Graphics, NULL, "Font height", "Gets current font's height")
{
	if(!Gr_inited)
		return LUA_RETURN_NIL;
	
	return lua_set_args(L, "i", gr_get_font_height());
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

LUA_FUNC(setTarget, l_Graphics, "[texture Texture]", "True if successful, false otherwise",
		"If texture is specified, sets current rendering surface to a texture."
		"Otherwise, sets rendering surface back to screen.")
{
	int idx = -1;
	lua_get_args(L, "|o", l_Texture.Get(&idx));

	bool b = bm_set_render_target(idx);

	return lua_set_args(L, "b", b);
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

LUA_FUNC(setOpacity, l_Graphics, "Opacity %, [Opacity Type]", NULL,
		 "Sets opacity for 2D image drawing functions to specified amount and type. Valid types are:"
		 "<br>None"
		 "<br>Filter")
{
	float f;
	char *s=NULL;
	int idx=-1;

	if(!lua_get_args(L, "f|s", &f, &s))
		return LUA_RETURN_NIL;

	if(f > 100.0f)
		f = 100.0f;
	if(f < 0.0f)
		f = 0.0f;

	if(s != NULL)
	{
		if(!stricmp(s, "Filter"))
			idx = GR_ALPHABLEND_FILTER;
		else
			idx = GR_ALPHABLEND_NONE;
	}

	lua_Opacity = f*0.01f;
	if(idx > -1)
		lua_Opacity_type = idx;

	return LUA_RETURN_NIL;
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

LUA_FUNC(drawCircle, l_Graphics, "Radius, x, y", NULL, "Draws a circle")
{
	if(!Gr_inited)
		return LUA_RETURN_NIL;

	int x,y,ra;

	if(!lua_get_args(L, "iii", &ra,&x,&y))
		return LUA_RETURN_NIL;

	gr_circle(x,y, ra, false);

	return LUA_RETURN_NIL;
}

LUA_FUNC(drawCurve, l_Graphics, "x, y, Radius, Direction", NULL, "Draws a curve")
{
	if(!Gr_inited)
		return LUA_RETURN_NIL;

	int x,y,ra,d;

	if(!lua_get_args(L, "iiii", &x,&y,&ra,&d))
		return LUA_RETURN_NIL;

	gr_curve(x,y,ra,d);

	return LUA_RETURN_NIL;
}

LUA_FUNC(drawGradientLine, l_Graphics, "x1, y1, x2, y2", NULL, "Draws a line that steadily fades out")
{
	if(!Gr_inited)
		return 0;

	int x1,y1,x2,y2;

	if(!lua_get_args(L, "iiii", &x1, &y1, &x2, &y2))
		return LUA_RETURN_NIL;

	gr_gradient(x1,y1,x2,y2,false);

	return LUA_RETURN_NIL;
}

LUA_FUNC(drawLine, l_Graphics, "x1, y1, x2, y2", NULL, "Draws a line with the current color")
{
	if(!Gr_inited)
		return LUA_RETURN_NIL;

	int x1,y1,x2,y2;

	if(!lua_get_args(L, "iiii", &x1, &y1, &x2, &y2))
		return LUA_RETURN_NIL;

	gr_line(x1,y1,x2,y2,false);

	return LUA_RETURN_NIL;
}

LUA_FUNC(drawPixel, l_Graphics, "x, y", NULL, "Sets pixel to current color")
{
	if(!Gr_inited)
		return LUA_RETURN_NIL;

	int x,y;

	if(!lua_get_args(L, "ii", &x, &y))
		return LUA_RETURN_NIL;

	gr_pixel(x,y,false);

	return LUA_RETURN_NIL;
}

LUA_FUNC(drawRectangle, l_Graphics, "x1, y1, x2, y2, [Filled]", NULL, "Draws a rectangle with the current color; default is filled")
{
	if(!Gr_inited)
		return LUA_RETURN_NIL;

	int x1,y1,x2,y2;
	bool f=true;

	if(!lua_get_args(L, "iiii|b", &x1, &y1, &x2, &y2, &f))
		return LUA_RETURN_NIL;

	if(f)
	{
		gr_rect(x1, y1, x2-x1, y2-y1, false);
	}
	else
	{
		gr_line(x1,y1,x2,y1,false);	//Top
		gr_line(x1,y2,x2,y2,false); //Bottom
		gr_line(x1,y1,x1,y2,false);	//Left
		gr_line(x2,y1,x2,y2,false);	//Right
	}

	return LUA_RETURN_NIL;
}

#define MAX_TEXT_LINES		256

LUA_FUNC(drawString, l_Graphics, "String, x1, y1, [x2, y2]", NULL, "Draws a string")
{
	if(!Gr_inited)
		return LUA_RETURN_NIL;

	int x,y;
	char *s;
	int x2=-1,y2=-1;

	if(!lua_get_args(L, "sii|ii", &s, &x, &y, &x2, &y2))
		return LUA_RETURN_NIL;

	if(x2 < 0) {
		gr_string(x,y,s,false);
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
			gr_string(x,y2,linestarts[i],false);

			//Set character back
			*reptr = rep;
		}

		delete[] linelengths;
		delete[] linestarts;
	}

	return LUA_RETURN_NIL;
}

LUA_FUNC(getStringWidth, l_Graphics, "String to get width of", "String width", "Gets string width")
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

LUA_FUNC(createTexture, l_Graphics, "Width, Height, Type", "Handle to new texture", "Creates a texture for rendering to. Types are static - for infrequent rendering - and dynamic - for frequent rendering.")
{
	int w,h;
	char *s;
	if(!lua_get_args(L, "iis", &w, &h, &s))
		return LUA_RETURN_NIL;

	int t = 0;
	if(!stricmp(s, "Static"))
		t = BMP_TEX_STATIC_RENDER_TARGET;
	else if(!stricmp(s, "Dynamic"))
		t = BMP_TEX_DYNAMIC_RENDER_TARGET;

	int idx = bm_make_render_target(w, h, t);

	if(idx < 0)
		return LUA_RETURN_NIL;

	return lua_set_args(L, "o", l_Texture.Set(idx));
}

LUA_FUNC(loadTexture, l_Graphics, "Texture filename, [Load if Animation, No drop frames]", "Texture handle, false if invalid name",
		 "Gets a handle to a texture. If second argument is set to true, animations will also be loaded."
		 "If third argument is set to true, every other animation frame will not be loaded if system has less than 48 MB memory."
		 "<br><strong>IMPORTANT:</strong> Textures will not be unload themselves unless you explicitly tell them to do so."
		 "When you are done with a texture, call the Unload() function to free up memory.")
{
	char *s;
	int idx;
	bool b=false;
	bool d=false;

	if(!lua_get_args(L, "s", &s, &b, &d))
		return LUA_RETURN_NIL;

	idx = bm_load(s);
	if(idx < 0 && b) {
		idx = bm_load_animation(s, NULL, NULL, d ? 1 : 0);
	}

	if(idx < 0)
		return LUA_RETURN_FALSE;

	return lua_set_args(L, "o", l_Texture.Set(idx));
}

LUA_FUNC(drawImage, l_Graphics, "Image name/Texture handle, x1, y1, [x2, y2, X start, Y start]", "Whether image or texture was drawn",
		 "Draws an image or texture. Any image extension passed will be ignored."
		 "Width and height to show specify the number of pixels from the left and top boundaries that will be drawn."
		 "X start and Y start specify the left and top bounaries, respectively.")
{
	if(!Gr_inited)
		return LUA_RETURN_NIL;

	int idx;
	int x,y;
	int x2=INT_MAX;
	int y2=INT_MAX;
	int sx=0;
	int sy=0;

	if(lua_isstring(L, 1))
	{
		char *s = NULL;
		if(!lua_get_args(L, "sii|iiii", &s,&x,&y,&x2,&y2,&sx,&sy))
			return LUA_RETURN_NIL;

		idx = Script_system.LoadBm(s);

		if(idx < 0)
			return LUA_RETURN_FALSE;
	}
	else
	{
		if(!lua_get_args(L, "oii|biiii", l_Texture.Get(&idx),&x,&y,&x2,&y2,&sx,&sy))
			return LUA_RETURN_NIL;
	}

	int w, h;
	if(bm_get_info(idx, &w, &h) < 0)
		return LUA_RETURN_FALSE;

	if(sx < 0)
		sx = w + sx;

	if(sy < 0)
		sy = h + sy;
	
	if(x2!=INT_MAX)
		w = x2-x;

	if(y2!=INT_MAX)
		h = y2-y;

	gr_set_bitmap(idx, lua_Opacity_type, GR_BITBLT_MODE_NORMAL,lua_Opacity);
	gr_bitmap_ex(x, y, w, h, sx, sy, false);

	return LUA_RETURN_TRUE;
}

LUA_FUNC(drawMonochromeImage, l_Graphics, "Image name/Texture handle, x1, y1, [x2, y2, X start, Y start, Mirror]", "Whether image was drawn", "Draws a monochrome image using the current color")
{
	if(!Gr_inited)
		return LUA_RETURN_NIL;

	int idx;
	int x,y;
	int x2=INT_MAX;
	int y2=INT_MAX;
	int sx=0;
	int sy=0;
	bool m = false;

	if(lua_isstring(L, 1))
	{
		char *s = NULL;
		if(!lua_get_args(L, "sii|iiiib", &s,&x,&y,&x2,&y2,&sx,&sy,&m))
			return LUA_RETURN_NIL;

		idx = Script_system.LoadBm(s);

		if(idx < 0)
			return LUA_RETURN_FALSE;
	}
	else
	{
		if(!lua_get_args(L, "oii|biiiib", l_Texture.Get(&idx),&x,&y,&x2,&y2,&sx,&sy,&m))
			return LUA_RETURN_NIL;
	}

	int w, h;
	if(bm_get_info(idx, &w, &h) < 0)
		return LUA_RETURN_FALSE;

	if(sx < 0)
		sx = w + sx;

	if(sy < 0)
		sy = h + sy;
	
	if(x2!=INT_MAX)
		w = x2-x;

	if(y2!=INT_MAX)
		h = y2-y;

	gr_set_bitmap(idx, lua_Opacity_type, GR_BITBLT_MODE_NORMAL,lua_Opacity);
	gr_aabitmap_ex(x, y, w, h, sx, sy, false, m);

	return LUA_RETURN_TRUE;
}

LUA_FUNC(getImageWidth, l_Graphics, "Image name", "Image width", "Gets image width")
{
	char *s;
	if(!lua_get_args(L, "s", &s))
		return LUA_RETURN_NIL;

	int w;
	
	int idx = bm_load(s);

	if(idx < 0)
		return LUA_RETURN_NIL;

	bm_get_info(idx, &w);
	return lua_set_args(L, "i", w);
}

LUA_FUNC(getImageHeight, l_Graphics, "Image name", "Image height", "Gets image height")
{
	char *s;
	if(!lua_get_args(L, "s", &s))
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

LUA_FUNC(playGameSound, l_SoundLib, "Sound filename, [Panning (-1.0 left to 1.0 right), Volume %, Priority 0-3, Voice Message?]", "True if sound was played, false if not (Replaced with a sound instance object in the future)", "Plays a sound from #Game Sounds in sounds.tbl. A priority of 0 indicates that the song must play; 1-3 will specify the maximum number of that sound that can be played")
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

//*************************Testing stuff*************************
//This section is for stuff that's considered experimental.
lua_lib l_Testing("Testing", "ts", "Experimental or testing stuff");

LUA_FUNC(createParticle, l_Testing, "vector Position, vector Velocity, number Lifetime, number Radius, string Type, [number Tracer length=-1, boolean Reverse=false, texture Texture=Nil, object Attached Object=Nil]", NULL,
		 "Creates a particle. Types are 'Debug', 'Bitmap', 'Fire', 'Smoke', 'Smoke2', and 'Persistent Bitmap'."
		 "Reverse reverse animation, if one is specified"
		 "Attached object specifies object that Position will be (and always be) relative to.")
{
	particle_info pi;
	pi.type = PARTICLE_DEBUG;
	pi.optional_data = 0;
	pi.tracer_length = 1.0f;
	pi.attached_objnum = -1;
	pi.attached_sig = -1;
	pi.reverse = 0;

	char *s=NULL;
	bool rev=false;
	object_h *objh=NULL;
	if(!lua_get_args(L, "ooffs|fboo", l_Vector.Get(&pi.pos), l_Vector.Get(&pi.vel), &pi.lifetime, &pi.rad, &s, &pi.tracer_length, &rev, l_Texture.Get((int*)&pi.optional_data), l_Object.GetPtr(&objh)))
		return LUA_RETURN_NIL;

	if(s != NULL)
	{
		if(!stricmp(s, "Debug"))
			pi.type = PARTICLE_DEBUG;
		else if(!stricmp(s, "Bitmap"))
			pi.type = PARTICLE_BITMAP;
		else if(!stricmp(s, "Fire"))
			pi.type = PARTICLE_FIRE;
		else if(!stricmp(s, "Smoke"))
			pi.type = PARTICLE_SMOKE;
		else if(!stricmp(s, "Smoke2"))
			pi.type = PARTICLE_SMOKE2;
		else if(!stricmp(s, "Persistent bitmap"))
			pi.type = PARTICLE_BITMAP_PERSISTENT;
	}

	if(rev)
		pi.reverse = 0;

	if(objh != NULL && objh->IsValid())
	{
		pi.attached_objnum = (short)OBJ_INDEX(objh->objp);
		pi.attached_sig = objh->objp->signature;
	}

	particle_create(&pi);

	return LUA_RETURN_NIL;
}

LUA_FUNC(createCamera, l_Testing, "string Name, [world vector Position, world orientation Orientation]", "camera Handle", "Creates a new camera")
{
	char *s = NULL;
	vec3d *v = NULL;
	matrix_h *mh = NULL;
	if(!lua_get_args(L, "s|oo", &s, l_Vector.GetPtr(&v), l_Matrix.GetPtr(&mh)))
		return LUA_RETURN_NIL;

	int idx;

	//Add camera
	Cameras.push_back(camera(s));

	//Get idx
	idx = Cameras.size() - 1;

	//Set pos/orient
	if(v != NULL)
		Cameras[idx].set_position(v);
	if(mh != NULL)
	{
		mh->ValidateMatrix();
		Cameras[idx].set_rotation(&mh->mtx);
	}

	//Set position
	return lua_set_args(L, "o", l_Camera.Set(idx));
}

LUA_FUNC(getCameraByName, l_Testing, "Camera name", "Camera handle", "Gets camera handle")
{
	char *name;
	if(!lua_get_args(L, "s", &name))
		return LUA_RETURN_NIL;

	int idx = cameras_lookup(name);
	
	if(idx < 0) {
		return LUA_RETURN_NIL;
	}

	return lua_set_args(L, "o", l_Camera.Set(idx));
}

LUA_FUNC(getNumCameras, l_Testing, NULL, "Number of cameras", "Gets number of cameras.")
{
	return lua_set_args(L, "i", (int)Cameras.size());
}

LUA_FUNC(getCameraByIndex, l_Testing, "Camera index", "Camera handle", "Gets camera handle")
{
	int i;
	if(!lua_get_args(L, "i", &i))
		return LUA_RETURN_NIL;

	if(i < 1 || (uint)i > Cameras.size())
		return LUA_RETURN_NIL;

	//Lua-->FS2
	i--;

	return lua_set_args(L, "o", l_Camera.Set(i));
}

LUA_FUNC(setCamera, l_Testing, "[camera handle Camera]", "True", "Sets current camera, or resets camera if none specified")
{
	int idx;
	if(!lua_get_args(L, "o", l_Camera.Get(&idx)))
	{
		Viewer_mode &= ~VM_FREECAMERA;
		return LUA_RETURN_NIL;
	}

	if(idx < 1 || (uint)idx > Cameras.size())
		return LUA_RETURN_NIL;

	Viewer_mode |= VM_FREECAMERA;
	Current_camera = &Cameras[idx];

	return LUA_RETURN_TRUE;
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

//Inits LUA
//Note that "libraries" must end with a {NULL, NULL}
//element
int script_state::CreateLuaState()
{
	mprintf(("LUA: Opening LUA state...\n"));
	lua_State *L = lua_open();   /* opens Lua */

	mprintf(("LUA: Initializing base Lua libraries...\n"));
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
			LuaError(L, "Couldn't create metatable for object '%s'", lib->Name);
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
}

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
			if(lib->Indexer != NULL) {
				draw_header = true;
				break;
			}

			func = &lib->Functions[0];
			func_end = &lib->Functions[lib->Functions.size()];
			for(; func < func_end; func++)
			{
				if(!strncmp(func->Name, "__", 2)) {
					draw_header = true;
					break;
				}
			}

			if(lib_deriv == NULL || draw_header)
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
				fprintf(fp, "<dt><b>%s</b> <i>%s</i></dt>", draw_name, func->Arguments);
			} else {
				fprintf(fp, "<dt><b>%s</b></dt>", draw_name);
			}

			if(func->Description != NULL) {
				fprintf(fp, "<dd>%s</dd>", func->Description);
			} else {
				fputs("<dd>No description</dd>", fp);
			}

			if(func->ReturnValues != NULL) {
				fprintf(fp, "<dd><b>Result:</b> %s<br>&nbsp;</dd>", func->ReturnValues);
			} else {
				fputs("<dd><b>Result:</b> None<br>&nbsp;</dd>", fp);
			}
		}

		if(lib_deriv == NULL)
			break;

		lib = lib_deriv;
	}
	if(draw_header) {
		fputs("</dl></dd>", fp);
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
			if(var->Type != NULL)
			{
				uint idx;
				for(idx = 0; idx < lua_Objects.size(); idx++)
				{
					if(!stricmp(lua_Objects[idx].Name, var->Type))
						break;
				}
				if(idx < lua_Objects.size())
					fprintf(fp, "<dt><a href=\"#%s\"><i>%s</i></a> <b>%s</b></dt>", var->Type, var->Type, var->Name);
				else
					fprintf(fp, "<dt><i>%s</i> <b>%s</b></dt>", var->Type, var->Name);
			}
			else
			{
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
	lib = main_lib;
	draw_header = false;
	if(main_lib->Functions.size() || (lib_deriv != NULL && lib_deriv->Functions.size()))
	{
		for(i = 0; i < 2; i++)
		{
			func = &lib->Functions[0];
			func_end = &lib->Functions[lib->Functions.size()];
			for(; func < func_end; func++)
			{
				if(strncmp(func->Name, "__", 2)) {
					draw_header = true;
					break;
				}
			}

			if(lib_deriv == NULL || draw_header)
				break;

			lib = lib_deriv;
		}
	}
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

void script_state::OutputLuaMeta(FILE *fp)
{
	//***Output Libraries
	fputs("<dl>", fp);
	fputs("<dt><b>Libraries</b></dt>", fp);
	lua_lib_h *lib = &lua_Libraries[0];
	lua_lib_h *lib_end = &lua_Libraries[lua_Libraries.size()];
	for(; lib < lib_end; lib++)
	{
		fprintf(fp, "<dd><a href=\"#%s\">%s (%s)</a> - %s</dd>", lib->Name, lib->Name, lib->ShortName, lib->Description);
	}

	//***Output objects
	lib = &lua_Objects[0];
	lib_end = &lua_Objects[lua_Objects.size()];
	fputs("<dt><b>Objects</b></dt>", fp);
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
}
