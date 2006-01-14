#include "parse/scripting.h"
#ifdef USE_LUA
#include <string.h>
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

//*************************Lua funcs*************************
int script_remove_lib(lua_State *L, char *name);

void lua_stackdump(lua_State *L);

//*************************Lua helpers*************************
//Used for internal object->lua_return and lua_parse->object communication
struct script_lua_odata
{
	char *meta;
	void *buf;
	int size;

	script_lua_odata(char *in_meta, void *in_buf, int in_size){meta=in_meta;buf=in_buf;size=in_size;}
};

//Used like _odata, except for object pointers
struct script_lua_opdata
{
	char *meta;
	void **buf;

	script_lua_opdata(char* in_meta, void** in_buf){meta=in_meta; buf=in_buf;}
};

//Function helper helper
//Stores data about functions
struct lua_func_hh
{
	char *Name;
	lua_CFunction Function;
	char *Arguments;
	char *ReturnValues;
	char *Description;
};

class lua_lib_h
{
/*
	friend class lua_lib;
	friend class lua_obj_h;
	friend int script_state::CreateLuaState();
	friend void output_lib_meta(FILE *fp, lua_lib_h *lib);
	friend void script_state::OutputLuaMeta(FILE *fp);
*/
//private:
public:
	char *Name;
	char *Description;
	std::vector<lua_func_hh> Functions;
	int Derivator;

public:
	lua_lib_h(char *in_name, char *in_desc, int in_deriv=-1){Name = in_name; Description = in_desc; Derivator = in_deriv;}
};

std::vector<lua_lib_h> lua_Libraries;
std::vector<lua_lib_h> lua_Objects;

//Library class
//This is what you define a variable of to make new libraryes
class lua_lib {
private:
	int lib_idx;
public:
	lua_lib(char *in_name, char *in_desc){
		lua_Libraries.push_back(lua_lib_h(in_name, in_desc));
		lib_idx = lua_Libraries.size()-1;
	}

	void AddFunc(lua_func_hh *f){lua_Libraries[lib_idx].Functions.push_back(*f);}
};

//Lua_obj helper class
//Because of some STUPID limitation with templates, I couldn't pass
//a lua_obj object as a reference and then execute AddFunc()
//This class is a hack via derivation to make it work -C
class lua_obj_h
{
public:
	int obj_idx;
public:
	lua_obj_h(char *in_name, char *in_desc, lua_obj_h *in_deriv = NULL) {
		lua_Objects.push_back(lua_lib_h(in_name, in_desc, in_deriv == NULL ? -1 : in_deriv->obj_idx));	//WMC - Handle NULL case
		obj_idx = lua_Objects.size()-1;
	}

	void AddFunc(lua_func_hh *f){lua_Objects[obj_idx].Functions.push_back(*f);}
};

//Function helper class
//Lets us add functions via its constructor
class lua_func_h {
public:
	lua_func_h(char *name, lua_CFunction func, lua_lib &lib, char *args=NULL, char *retvals=NULL, char *desc=NULL) {
		lua_func_hh f;

		f.Name = name;
		f.Function = func;
		f.Description = desc;
		f.Arguments = args;
		f.ReturnValues = retvals;

		lib.AddFunc(&f);
	}

	lua_func_h(char *name, lua_CFunction func, lua_obj_h &obj, char *args=NULL, char *retvals=NULL, char *desc=NULL) {
		lua_func_hh f;

		f.Name = name;
		f.Function = func;
		f.Description = desc;
		f.Arguments = args;
		f.ReturnValues = retvals;

		obj.AddFunc(&f);
	}
};

//Object class
//This is what you define a variable of to make new objects
template <class StoreType> class lua_obj : public lua_obj_h
{
public:
	lua_obj(char*in_name, char*in_desc, lua_obj* in_deriv=NULL):lua_obj_h(in_name, in_desc, in_deriv){};

	//StoreType *Create(lua_State *L){StoreType *ptr = (StoreType*)LUA_NEW_OBJ(L, meta, StoreType); return ptr;}
	script_lua_odata SetToLua(StoreType *obj){return script_lua_odata(lua_Objects[obj_idx].Name, obj, sizeof(StoreType));}
	script_lua_odata GetFromLua(StoreType *ptr){return script_lua_odata(lua_Objects[obj_idx].Name, ptr, sizeof(StoreType));}
	script_lua_opdata GetPtrFromLua(StoreType **ptr){return script_lua_opdata(lua_Objects[obj_idx].Name, ptr);}
};

//Function macro
//This is what you call to make new functions
#define LUA_FUNC(name, objlib, args, retvals, desc)	\
	static int lua_##objlib##_##name(lua_State *L);	\
	lua_func_h lua_##objlib##_##name##_h(#name, lua_##objlib##_##name, objlib, args, retvals, desc);	\
	static int lua_##objlib##_##name(lua_State *L)
/*
//Callback typedef
#define SCRIPT_LUA_CALLBACK static int
//Function entry for a library
typedef struct script_lua_func_list {
	const char *name;
	lua_CFunction func;
	const char *description;
	const char *args;
	const char *retvals;
} script_lua_func_list;

//Function entry for an object
typedef struct script_lua_obj_func_list {
	const char *name;
	lua_CFunction func;
	const char *description;
	const char *args;
	const char *retvals;
} script_lua_obj_func_list;

//Object entry for a library
typedef struct script_lua_obj_list
{
	const char *object_name;
	const script_lua_obj_func_list *object_funcs;
	const char *description;
}script_lua_obj_list;

//Library entry
typedef struct script_lua_lib_list
{
	const char *library_name;
	const script_lua_func_list *library_funcs;
	const script_lua_obj_list *library_objects;
	const char *description;
}script_lua_lib_list;

//LUA_NEW_OBJ(lua_State, object name, object struct)
#define LUA_NEW_OBJ(L,n,s)					\
	(s *) lua_newuserdata(L, sizeof(s));	\
	luaL_getmetatable(L, n);				\
	lua_setmetatable(L, -2)

template <class StoreType> class lua_cvar
{
	char meta[NAME_LENGTH];
public:
	lua_cvar(char *in_meta){sprintf(meta, "fs2.%s", in_meta);}

	char *GetName(){return meta;}
	//StoreType *Create(lua_State *L){StoreType *ptr = (StoreType*)LUA_NEW_OBJ(L, meta, StoreType); return ptr;}
	script_lua_odata Return(StoreType *obj){return script_lua_odata(meta, obj, sizeof(StoreType));}
	script_lua_odata GetFromLua(StoreType *ptr){return script_lua_odata(meta, ptr, sizeof(StoreType));}
	script_lua_opdata ParsePtr(StoreType **ptr){return script_lua_opdata(meta, ptr);}
};
*/
/*struct script_lua_pdata
{
	char *meta;
	void *buf;
	
	script_lua_pdata(char* in_meta, void* in_buf){meta=in_meta;buf=in_buf;}
};*/

//LUA_CVAR(name, pointer)
//Call this with lua_get_args to store the pointer to
//a specific set of userdata in a pointer.
//-----EX:
//lua_get_args(L, "u", LUA_CVAR("ship", &lua_ship))
#define LUA_CVAR(n, p) script_lua_udata(n, (void**)&p)

//#define LUA_PTR(n, p) script_lua_pdata(n, (void*)p)
//*************************Lua return values*************************
#define LUA_RETURN_NIL		0
#define LUA_RETURN_OBJECT		1
#define LUA_RETURN_TRUE		lua_set_args(L, "b", true)
#define LUA_RETURN_FALSE	lua_set_args(L, "b", true)

//*************************Lua defines*************************
//These are the various types of operators you can
//set in Lua. Define these in script_lua_obj_func_list
#define	LUA_OPER_METHOD				"__index"
#define	LUA_OPER_ADDITION			"__add"
#define LUA_OPER_SUBTRACTION		"__sub"
#define LUA_OPER_MULTIPLICATION		"__mult"
#define LUA_OPER_DIVISION			"__div"
#define LUA_OPER_POWER				"__pow"
#define LUA_OPER_UNARY				"__unm"
#define LUA_OPER_CONCATENATION		"__concat"
#define LUA_OPER_EQUALTO			"__eq"
#define LUA_OPER_LESSTHAN			"__lt"
#define LUA_OPER_LESSTHANEQUALTO	"__le"
#define LUA_OPER_INDEX				"__index"
#define LUA_OPER_NEWINDEX			"__newindex"
#define LUA_OPER_CALL				"__call"
#define LUA_OPER_TOSTRING			"__tostring"

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
	void *v;
	lua_State *ls;
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
				v = lua_touserdata(L, argnum);
				sprintf(buf, "Userdata [%d]", v);
				strcat(stackdump, buf);
				break;
			case LUA_TTHREAD:
				ls = lua_tothread(L, argnum);
				sprintf(buf, "Thread [%d]", ls);
				strcat(stackdump, buf);
				break;
			case LUA_TLIGHTUSERDATA:
				v = lua_touserdata(L, argnum);
				sprintf(buf, "Light userdata [%d]", v);
				strcat(stackdump, buf);
				break;
			default:
				sprintf(buf, "<UNKNOWN>: %s (%d) (%s)", lua_typename(L, type), lua_tonumber(L, argnum), lua_tostring(L, argnum));
				strcat(stackdump, buf);
				break;
		}
	}
}

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
		Warning(LOCATION, "Not enough arguments for function");
		return 0;
	}

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
					Warning(LOCATION, "Argument %d is an invalid type; boolean expected", nargs);
					if(!optional_args) return 0;
				}
				break;
			case 'd':
				if(lua_isnumber(L, nargs)) {
					*va_arg(vl, double*) = (double)lua_tonumber(L, nargs);
				} else {
					Warning(LOCATION, "Argument %d is an invalid type; float expected", nargs);
					if(!optional_args) return 0;
				}
				break;
			case 'f':
				if(lua_isnumber(L, nargs)) {
					*va_arg(vl, float*) = (float)lua_tonumber(L, nargs);
				} else {
					Warning(LOCATION, "Argument %d is an invalid type; float expected", nargs);
					if(!optional_args) return 0;
				}
				break;
			case 'i':
				if(lua_isnumber(L, nargs)) {
					*va_arg(vl, int*) = (int)lua_tonumber(L, nargs);
				} else {
					Warning(LOCATION, "Argument %d is an invalid type; int expected", nargs);
					if(!optional_args) return 0;
				}
				break;
			case 's':
				if(lua_isstring(L, nargs)) {
					*va_arg(vl, const char **) = lua_tostring(L, nargs);
				} else {
					Warning(LOCATION, "Argument %d is an invalid type; string expected", nargs);
					if(!optional_args) return 0;
				}
				break;
			case 'o':
				if(lua_isuserdata(L, nargs))
				{
					script_lua_odata od = va_arg(vl, script_lua_odata);
					void *ptr = luaL_checkudata(L, nargs, od.meta);
					if(ptr == NULL) {
						Warning(LOCATION, "Argument %d is the wrong type of userdata; %s expected", nargs, od.meta);
					}
					memcpy(od.buf, ptr, od.size);
				}
				else
				{
					Warning(LOCATION, "Argument %d is an invalid type %d; type '%s' expected", nargs, lua_type(L, nargs), va_arg(vl, script_lua_odata).meta);
					if(!optional_args) return 0;
				}
				break;
			case 'p':
				if(lua_isuserdata(L, nargs))
				{
					script_lua_opdata pd = va_arg(vl, script_lua_opdata);
					(*pd.buf) = luaL_checkudata(L, nargs, pd.meta);
					if((*pd.buf) == NULL) {
						Warning(LOCATION, "Argument %d is the wrong type of userdata; %s expected", nargs, pd.meta);
					}
				}
				else
				{
					Warning(LOCATION, "Argument %d is an invalid type %d; type '%s' expected", nargs, lua_type(L, nargs), va_arg(vl, script_lua_opdata).meta);
					if(!optional_args) return 0;
				}
				break;
			case '|':
				nargs--;	//cancel out the nargs++ at the end
				optional_args = true;
				break;
			default:
				Error(LOCATION, "Bad character passed to lua_get_args; (%c)", *(fmt-1));
				break;
		}
		nargs++;
	}
	va_end(vl);
	return nargs;
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
	while(*fmt)
	{
		switch(*fmt++)
		{
			case 'b':
				lua_pushboolean(L, va_arg(vl, bool) ? 1 : 0);
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
			case 'o':	//Just let nargs increment; it should already be on the stack
				{
					script_lua_odata od = va_arg(vl, script_lua_odata);
					void *newod = (void*)lua_newuserdata(L, sizeof(void*));
					luaL_getmetatable(L, od.meta);
					lua_setmetatable(L, -2);
					memcpy(newod, od.buf, od.size);
					break;
				}
			default:
				Error(LOCATION, "Bad character passed to lua_set_args; (%c)", *fmt);
		}
		nargs++;
	}
	va_end(vl);
	return nargs;
}
//*************************Object #defines*************************
//While you don't _need_ to add object defines here, it's a good idea
//so you can catch misspellings and make sure the name isn't taken already
//note that all object names should start with "fs2." to prevent conflicts
//
//IMPORTANT: Before you can use a type, it MUST be defined in a library
//				that exists in the current state
/*
static lua_cvar<vec3d>			l_lvec("lvec");
static lua_cvar<vec3d>			l_wvec("wvec");
static lua_cvar<polymodel*>		l_model("model");
static lua_cvar<ship_info*>		l_ship_info("ship_info");
static lua_cvar<ship*>			l_ship("ship");
*/


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

//**********CLASS: Shipclass
lua_obj<int> l_Shipclass("shipclass", "Ship class object");
extern int ships_inited;

LUA_FUNC(getName, l_Shipclass, NULL, "Name (string)", "Gets ship class name")
{
	int idx;
	if(!lua_get_args(L, "o", l_Shipclass.GetFromLua(&idx)))
		return LUA_RETURN_NIL;

	if(idx < 0 || idx > Num_ship_classes)
		return LUA_RETURN_NIL;

	return lua_set_args(L, "s", Ship_info[idx].name);
}

LUA_FUNC(getType, l_Shipclass, NULL, "Ship type (string)", "Gets ship type name")
{
	int idx;
	if(!lua_get_args(L, "o", l_Shipclass.GetFromLua(&idx)))
		return LUA_RETURN_NIL;

	if(idx < 0 || idx > Num_ship_classes)
		return LUA_RETURN_NIL;

	return lua_set_args(L, "s", Ship_types[Ship_info[idx].class_type].name);
}

LUA_FUNC(getSpecies, l_Shipclass, NULL, "Species name (string)", "Gets ship species")
{
	int idx;
	if(!lua_get_args(L, "o", l_Shipclass.GetFromLua(&idx)))
		return LUA_RETURN_NIL;

	if(idx < 0 || idx > Num_ship_classes)
		return LUA_RETURN_NIL;

	return lua_set_args(L, "s", Species_info[Ship_info[idx].species].species_name);
}

LUA_FUNC(getShortName, l_Shipclass, NULL, "Short name (string)", "Gets ship class short name")
{
	int idx;
	if(!lua_get_args(L, "o", l_Shipclass.GetFromLua(&idx)))
		return LUA_RETURN_NIL;

	if(idx < 0 || idx > Num_ship_classes)
		return LUA_RETURN_NIL;

	return lua_set_args(L, "s", Ship_info[idx].short_name);
}

LUA_FUNC(getHitpoints, l_Shipclass, NULL, "Hitpoints (number)", "Gets ship class hitpoints")
{
	int idx;
	if(!lua_get_args(L, "o", l_Shipclass.GetFromLua(&idx)))
		return LUA_RETURN_NIL;

	if(idx < 0 || idx > Num_ship_classes)
		return LUA_RETURN_NIL;

	return lua_set_args(L, "f", Ship_info[idx].max_hull_strength);
}

LUA_FUNC(isInTechroom, l_Shipclass, NULL, "Whether ship has been revealed in the techroom", "Gets whether or not the ship class is available in the techroom")
{
	int idx;
	if(!lua_get_args(L, "o", l_Shipclass.GetFromLua(&idx)))
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


LUA_FUNC(renderTechModel, l_Shipclass, "X1, Y1, X2, Y2, [Resize], [Rotation %], [Pitch radians], [Bank radians]", "Whether ship was rendered", "Draws ship model as if in techroom")
{
	int x1,y1,x2,y2;
	float rot_pct = 40.0f;
	angles rot_angles = {0.0f, 0.0f, 0.0f};
	bool r;
	int idx;
	if(!lua_get_args(L, "oiiii|bffff", l_Shipclass.GetFromLua(&idx), &x1, &y1, &x2, &y2, &r, &rot_pct, &rot_angles.b, &rot_angles.b))
		return LUA_RETURN_NIL;

	if(idx < 0 || idx > Num_ship_classes)
		return lua_set_args(L, "b", false);

	if(x2 < x1 || y2 < y1)
		return lua_set_args(L, "b", false);

	if(rot_pct < 0.0f)
		rot_pct = 0.0f;
	if(rot_pct > 100.0f)
		rot_pct = 100.0f;

	ship_info *sip = &Ship_info[idx];

	//Make sure model is loaded
	sip->modelnum = model_load(sip->pof_file, sip->n_subsystems, &sip->subsystems[0], 0);

	if(sip->modelnum < 0)
		return lua_set_args(L, "b", false);

	//Handle angles
	matrix orient = vmd_identity_matrix;
	angles view_angles = {-0.6f, 0.0f, 0.0f};
	vm_angles_2_matrix(&orient, &view_angles);

	rot_angles.h = (rot_pct*0.01f) * PI2;
	vm_rotate_matrix_by_angles(&orient, &rot_angles);

	//Clip
	gr_set_clip(x1,y1,x2-x1,y2-y1,r);

	//Handle 3D init stuff
	g3_start_frame(1);
	g3_set_view_matrix(&sip->closeup_pos, &vmd_identity_matrix, sip->closeup_zoom * 1.3f);
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

//**********CLASS: Object
lua_obj<int> l_Object("object", "Object");
//Helper function
//Returns 1 if object sig stored in idx exists, and stores Objects[] index in idx
//Returns 0 if object sig does not exist, and does not change idx
int lua_obj_get_idx(lua_State *L, int *idx)
{
	if(!lua_get_args(L, "o", idx))
		return 0;

	*idx = obj_get_by_signature(*idx);

	if(*idx < 0)
		return 0;

	return 1;
}

LUA_FUNC(getHitpointsLeft, l_Object, NULL, "Current hull hitpoints (number)", "Gets ship hull")
{
	int idx;
	if(!lua_obj_get_idx(L, &idx))
		return LUA_RETURN_NIL;

	return lua_set_args(L, "f", Objects[idx].hull_strength);
}

//**********CLASS: Ship
lua_obj<int> l_Ship("ship", "Ship object", &l_Object);

//Helper function
//Returns 1 if object sig stored in idx exists, and stores Ships[] index in idx
//Returns 0 if object sig does not exist, and does not change idx
int lua_ship_get_idx(lua_State *L, int *idx)
{
	if(!lua_get_args(L, "o", l_Ship.GetFromLua(idx)))
		return 0;

	*idx = ship_get_by_signature(*idx);

	if(*idx < 0)
		return 0;

	return 1;
}


LUA_FUNC(getName, l_Ship, NULL, "ship name (string)", "Gets ship name")
{
	int idx;
	if(!lua_ship_get_idx(L, &idx))
		return LUA_RETURN_NIL;

	return lua_set_args(L, "s", Ships[idx].ship_name);
}

LUA_FUNC(getHitpoints, l_Ship, NULL, "max hull hitpoints (number)", "Gets ship hull max")
{
	int idx;
	if(!lua_ship_get_idx(L, &idx))
		return LUA_RETURN_NIL;

	return lua_set_args(L, "f", Ship_info[Ships[idx].ship_info_index].max_hull_strength);
}

LUA_FUNC(getClass, l_Ship, NULL, "Ship class handle (shipclass)", "Gets ship class handle")
{
	int idx;
	if(!lua_ship_get_idx(L, &idx))
		return LUA_RETURN_NIL;

	return lua_set_args(L, "o", l_Shipclass.SetToLua(&Objects[Ships[idx].objnum].signature));
}

LUA_FUNC(setAfterburnerFuel, l_Ship, "[Fuel amount]", "[New] fuel amount", "Returns ship fuel amount, or sets it if amount is specified")
{
	int idx;
	float fuel = -1.0f;
	if(!lua_get_args(L, "of", l_Ship.GetFromLua(&idx), &fuel))
		return 0;

	idx = ship_get_by_signature(idx);

	if(idx < 0)
		return LUA_RETURN_NIL;

	if(fuel >= 0.0f)
		Ships[idx].afterburner_fuel = fuel;

	return lua_set_args(L, "f", Ships[idx].afterburner_fuel);
}

LUA_FUNC(warpIn, l_Ship, NULL, NULL, "Warps ship in")
{
	int idx;
	if(!lua_ship_get_idx(L, &idx))
		return LUA_RETURN_NIL;

	shipfx_warpin_start(&Objects[Ships[idx].objnum]);

	return LUA_RETURN_NIL;
}

LUA_FUNC(warpOut, l_Ship, NULL, NULL, "Warps ship out")
{
	int idx;
	if(!lua_ship_get_idx(L, &idx))
		return LUA_RETURN_NIL;

	shipfx_warpout_start(&Objects[Ships[idx].objnum]);

	return LUA_RETURN_NIL;
}

//**********OBJECT: Player
lua_obj<int> l_Player("player", "Player object");

int player_helper(lua_State *L, int *idx)
{
	if(!lua_get_args(L, "o", l_Player.GetFromLua(idx)))
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
lua_lib l_Base("bs", "Base library");

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
	return lua_set_args(L, "o", l_Player.SetToLua(&idx));
}

LUA_FUNC(getPlayerByIndex, l_Base, "Player index", "Player object", "Gets the named player")
{
	if(Player == NULL)
		return LUA_RETURN_NIL;

	int idx;
	if(!lua_get_args(L, "i", &idx))
		return LUA_RETURN_NIL;

	if(idx < 0 || idx > Player_num)
		return LUA_RETURN_NIL;

	return lua_set_args(L, "o", l_Player.SetToLua(&idx));
}

LUA_FUNC(getNumPlayers, l_Base, NULL, "Number of players", "Gets the number of currently loaded players")
{
	return lua_set_args(L, "i", Player_num);
}


//**********LIBRARY: Math
lua_lib l_Math("ma", "Math library");

LUA_FUNC(getRandomNumber, l_Math, "[Smallest number], [Largest number]", "Random number", "Returns a random number; default is 0 to 1")
{
	float min = 0.0f;
	float max = 1.0f;
	lua_get_args(L, "ff", &min, &max);

	if(max < min)
		return LUA_RETURN_NIL;

	return lua_set_args(L, "f", frand_range(min, max));
}


//**********LIBRARY: Campaign
lua_lib l_Campaign("cn", "Campaign Library");

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

	return lua_set_args(L, "o", l_Cmission.SetToLua(&Campaign.next_mission));
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

	return lua_set_args(L, "o", l_Cmission.SetToLua(&Campaign.prev_mission));
}

LUA_FUNC(getMissionByName, l_Campaign, "Mission name", "Cmission object, or false if mission does not exist", "Gets the specified mission from the campaign by its name")
{
	char *s;

	if(!lua_get_args(L, "s", &s))
		return LUA_RETURN_NIL;

	for(int idx = 0; idx < Campaign.num_missions; idx++)
	{
		if(!stricmp(Campaign.missions[idx].name, s))
			return lua_set_args(L, "o", l_Cmission.SetToLua(&idx));
	}

	return LUA_RETURN_FALSE;
}


LUA_FUNC(getMissionByIndex, l_Campaign, "Mission number (Zero-based index)", "Cmission object", "Gets the specified mission by its index in the campaign")
{
	int idx;

	if(!lua_get_args(L, "i", &idx))
		return LUA_RETURN_NIL;

	if(idx < 0 || idx > Campaign.num_missions)
		return LUA_RETURN_NIL;

	return lua_set_args(L, "o", l_Cmission.SetToLua(&idx));
}


//**********LIBRARY: Mission
lua_lib l_Mission("mn", "Mission library");

//WMC - These are in freespace.cpp
extern void game_level_init(int seed = -1);
extern void game_post_level_init();
extern void game_render_frame_setup(vec3d *eye_pos, matrix *eye_orient);
extern void game_render_frame(vec3d *eye_pos, matrix *eye_orient);
extern void game_simulation_frame();
extern void game_update_missiontime();
extern void game_render_post_frame();
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

	return lua_set_args(L, "o", l_Shipclass.SetToLua(&Objects[Ships[idx].objnum].signature));
}

LUA_FUNC(getNumEscortShips, l_Mission, NULL, "Ship object", "Gets escort ship")
{
	return lua_set_args(L, "i", hud_escort_num_ships_on_list());
}

LUA_FUNC(getEscortShip, l_Mission, "Escort index", "Ship object", "Gets escort ship")
{
	int idx;
	if(!lua_get_args(L, "i", &idx))
		return 0;

	if(idx < 0)
		return LUA_RETURN_NIL;

	idx = hud_escort_return_objnum(idx);
	
	if(idx < 0)
		return LUA_RETURN_NIL;

	return lua_set_args(L, "o", l_Ship.SetToLua(&Objects[idx].signature));
}

//**********LIBRARY: Tables
lua_lib l_Tables("tb", "Tables library");

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
	
	if(idx < 0 || idx > Num_ship_classes) {
		return LUA_RETURN_NIL;
	}

	return lua_set_args(L, "o", l_Shipclass.SetToLua(&idx));
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

	return lua_set_args(L, "o", l_Shipclass.SetToLua(&idx));
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
lua_lib l_Mouse("ms", "Mouse library");

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
lua_lib l_Graphics("gr", "Graphics Library");

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

LUA_FUNC(drawText, l_Graphics, "String, x, y, [Resize]", NULL, "Draws a string")
{
	if(!Gr_inited)
		return LUA_RETURN_NIL;

	int x,y;
	char *s;
	bool r=true;

	if(!lua_get_args(L, "sii|b", &s, &x, &y, &r))
		return LUA_RETURN_NIL;

	gr_string(x,y,s,r);

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

LUA_FUNC(drawCircle, l_Graphics, "x, y, Radius, [Resize]", NULL, "Draws a circle")
{
	if(!Gr_inited)
		return LUA_RETURN_NIL;

	int x,y,ra;
	bool r=true;

	if(!lua_get_args(L, "iii|b", &x,&y,&ra,&r))
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

LUA_FUNC(drawMonochromeImage, l_Graphics, "Image name, x, y, [Width to show], [Height to show], [X start], [Y start], [Resize], [Mirror]", "Whether image was drawn", "Draws a monochrome image using the current color")
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

	if(!lua_get_args(L, "sii|iiiibb", &s,&x,&y,&w,&h,&sx,&sy,&r,&m))
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

LUA_FUNC(drawImage, l_Graphics, "Image name, x, y, [Resize]", "Whether image was drawn", "Draws an image")
{
	if(!Gr_inited)
		return LUA_RETURN_NIL;

	char *s;
	int x,y;
	bool r = true;

	if(!lua_get_args(L, "sii|b", &s,&x,&y,&r))
		return LUA_RETURN_NIL;

	int idx = bm_load(s);

	if(idx < 0)
		return lua_set_args(L, "b", false);

	gr_set_bitmap(idx);
	gr_bitmap(x, y, r);

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
lua_lib l_SoundLib("sd", "Sound Library");

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

//*************************Libraries*************************
/*
// **********LIBRARY: Base
// *****CLASS: (l)vec
SCRIPT_LUA_CALLBACK lua_fs2_lvec(lua_State *L)
{
	vec3d v3 = vmd_zero_vector;
	lua_get_args(L, "|fff", &v3.xyz.x, &v3.xyz.y, &v3.xyz.z);

	return lua_set_args(L, "o", l_lvec.SetToLua(&v3));
}

SCRIPT_LUA_CALLBACK lua_fs2_vec__addition(lua_State *L)
{
	vec3d v3a, v3b, v3r;
	if(!lua_get_args(L, "oo", l_lvec.GetFromLua(&v3a), l_lvec.GetFromLua(&v3b)))
		return 0;

	vm_vec_add(&v3r, &v3a, &v3b);

	return lua_set_args(L, "o", l_lvec.SetToLua(&v3r));
}

SCRIPT_LUA_CALLBACK lua_fs2_vec__subtraction(lua_State *L)
{
	vec3d v3a, v3b, v3r;
	if(!lua_get_args(L, "oo", l_lvec.GetFromLua(&v3a), l_lvec.GetFromLua(&v3b)))
		return 0;

	vm_vec_sub(&v3r, &v3a, &v3b);

	return lua_set_args(L, "o", l_lvec.SetToLua(&v3r));
}

SCRIPT_LUA_CALLBACK lua_fs2_vec__multiplication(lua_State *L)
{
	vec3d v3a, v3b, v3r;
	float f;
	if(lua_get_args(L, "of", l_lvec.GetFromLua(&v3a), &f))
	{
		vm_vec_copy_scale(&v3r, &v3a, f);

		return lua_set_args(L, "o", l_lvec.SetToLua(&v3r));
	}
	else if(lua_get_args(L, "oo", l_lvec.GetFromLua(&v3a), l_lvec.GetFromLua(&v3b)))
	{
		v3r.xyz.x = v3a.xyz.x * v3b.xyz.x;
		v3r.xyz.y = v3a.xyz.y * v3b.xyz.y;
		v3r.xyz.z = v3a.xyz.z * v3b.xyz.z;

		return lua_set_args(L, "o", l_lvec.SetToLua(&v3r));
	}

	//Neither is valid...

	return LUA_RETURN_NIL;
}


SCRIPT_LUA_CALLBACK lua_fs2_vec__tostring(lua_State *L)
{
	vec3d v3p;
	if(!lua_get_args(L, "o", l_lvec.GetFromLua(&v3p)))
		return 0;

	char buf[32];
	sprintf(buf, "(%f, %f, %f)", v3p.xyz.x, v3p.xyz.y, v3p.xyz.z);

	return lua_set_args(L, "s", buf);
}

SCRIPT_LUA_CALLBACK lua_fs2_vec__index(lua_State *L)
{
	vec3d v3p;
	int idx=0;
	if(!lua_get_args(L, "o|i", l_lvec.GetFromLua(&v3p), &idx))
		return 0;

	if(idx < 0 || idx > 2) {
		return 0;
	}

	return lua_set_args(L, "f", v3p.a1d[idx]);
}

SCRIPT_LUA_CALLBACK lua_fs2_vec_print(lua_State *L)
{
	vec3d v3p;
	int idx=0;
	if(!lua_get_args(L, "o|i", l_lvec.GetFromLua(&v3p), &idx))
		return 0;

	if(idx < 0 || idx > 2) {
		return 0;
	}

	//Error(LOCATION, "%f %f %f", v3p.xyz.x, v3p.xyz.y, v3p.xyz.z);

	return 0;
}

static const script_lua_obj_func_list lua_fs2_lvec_funcs[] = {
	{LUA_OPER_ADDITION, lua_fs2_vec__addition, "Adds two vectors"},
	{LUA_OPER_SUBTRACTION, lua_fs2_vec__subtraction, "Subtracts one vector from another"},
	{LUA_OPER_MULTIPLICATION, lua_fs2_vec__multiplication, "Multiplies two vectors, or a vector times a number"},
	{LUA_OPER_TOSTRING, lua_fs2_vec__tostring, "Converts vector to string"},
	{"print", lua_fs2_vec_print, "Prints a vector"},
	//{LUA_OPER_INDEX, lua_fs2_vec__index, "Returns index into vector"},
	{SCRIPT_END_LIST},
};

// *****CLASS: wvec
SCRIPT_LUA_CALLBACK lua_fs2_wvec(lua_State *L)
{
	vec3d v3 = vmd_zero_vector;
	lua_get_args(L, "|fff", &v3.xyz.x, &v3.xyz.y, &v3.xyz.z);

	vec3d v3p;
	v3p.xyz.x = v3.xyz.x;
	v3p.xyz.y = v3.xyz.y;
	v3p.xyz.z = v3.xyz.z;

	return lua_set_args(L, "o", l_wvec.SetToLua(&v3p));
}

static const script_lua_obj_func_list lua_fs2_wvec_funcs[] = {
	{LUA_OPER_ADDITION, lua_fs2_vec__addition, "Adds two vectors"},
	{LUA_OPER_SUBTRACTION, lua_fs2_vec__subtraction, "Subtracts one vector from another"},
	{LUA_OPER_MULTIPLICATION, lua_fs2_vec__multiplication, "Multiplies two vectors, or a vector times a number"},
	{LUA_OPER_TOSTRING, lua_fs2_vec__tostring, "Converts vector to string"},
	//{LUA_OPER_INDEX, lua_fs2_vec__index, "Returns index into vector"},
	{SCRIPT_END_LIST},
};

// *****LIBRARY CLASSES

static const script_lua_obj_list lua_base_lib_obj[] = {
	{l_lvec.GetName(), lua_fs2_lvec_funcs, "Local vector"},
	{l_wvec.GetName(), lua_fs2_wvec_funcs, "World vector"},
	{SCRIPT_END_LIST},
};

// *****LIBRARY FUNCTIONS

SCRIPT_LUA_CALLBACK lua_fs2_print(lua_State *L)
{
	Error(LOCATION, "LUA: %s", lua_tostring(L, -1));

	return 0;
}

SCRIPT_LUA_CALLBACK lua_fs2_error(lua_State *L)
{
	Error(LOCATION, "LUA ERROR: %s", lua_tostring(L, -1));

	return 0;
}

SCRIPT_LUA_CALLBACK lua_fs2_getState(lua_State *L)
{
	int depth = 0;
	lua_get_args(L, "|i", &depth);

	return lua_set_args(L, "s", GS_state_text[gameseq_get_state(depth)]);
}

static const script_lua_func_list lua_base_lib[] = {
	{"lvec", lua_fs2_lvec, "Creates a new local vector: (x, y, z)"},
	{"wvec", lua_fs2_wvec, "Creates a new world vector: (x, y, z)"},
	{"print", lua_fs2_print, "Displays output"},
	{"error", lua_fs2_error, "Causes an error"},
	{"getState", lua_fs2_getState, "Returns the current FS2 state string"},
	{SCRIPT_END_LIST},
};

// **********LIBRARY: Mission (msn)

SCRIPT_LUA_CALLBACK lua_msn_getShipInfo(lua_State *L)
{
	char *ship_name = NULL;
	if(!lua_get_args(L, "s", &ship_name))
		return 0;

	int idx = ship_info_lookup(ship_name);

	if(idx < 0)
		return 0;

	ship_info *sip = &Ship_info[idx];

	return lua_set_args(L, "o", l_ship_info.SetToLua(&sip));
}

SCRIPT_LUA_CALLBACK lua_msn_ShipInfo_print(lua_State *L)
{
	ship_info *sip;
	if(!lua_get_args(L, "o", l_ship_info.GetFromLua(&sip)))
		return 0;

	//Error(LOCATION, "%s", sip->name);

	return 0;
}

SCRIPT_LUA_CALLBACK lua_msn_newShip(lua_State *L)
{
	char *ship_name = NULL;
	char *class_name = NULL;
	vec3d pos;
	angles ang = {0,0,0};

	if(!lua_get_args(L, "ssfff|fff", &ship_name, &class_name, &pos.xyz.x, &pos.xyz.y, &pos.xyz.z, 
		&ang.p, &ang.b, &ang.h))
		return 0;

	//New matrix
	matrix ori = IDENTITY_MATRIX;
	vm_angles_2_matrix(&ori, &ang);

	//Find the class index
	int si_idx = ship_info_lookup(class_name);
	if(si_idx < 0)
	{
		//couldn't find it
		return LUA_RETURN_NIL;
	}

	//Create the ship
	int s_idx = ship_create(&ori, &pos, si_idx, ship_name);

	//Make the lua ship object, if the ship was created
	if(s_idx > -1)
	{
		//Make the lua ship object and set the ptr to NULL
		ship *shipp = &Ships[s_idx];
		return lua_set_args(L, "o", l_ship.SetToLua(&shipp));
	}

	return LUA_RETURN_NIL;
}

SCRIPT_LUA_CALLBACK lua_msn_ship_setSpeed(lua_State *L)
{
	ship *shipp = NULL;
	vec3d vel;

	if(!lua_get_args(L, "offf", l_ship.GetFromLua(&shipp), &vel.xyz.x, &vel.xyz.y, &vel.xyz.z))
		return LUA_RETURN_NIL;

	//Set the speed
	Objects[shipp->objnum].phys_info.vel = vel;

	//We're done!
	return LUA_RETURN_NIL;
}

SCRIPT_LUA_CALLBACK lua_msn_ship_setName(lua_State *L)
{
	ship *shipp = NULL;
	char *name = NULL;

	if(!lua_get_args(L, "os", l_ship.GetFromLua(&shipp), &name))
		return LUA_RETURN_NIL;

	//Set the name
	if(strlen(name)) {
		strcpy(shipp->ship_name, name);
	} else {
		sprintf(shipp->ship_name, "Ship %d", SHIP_INDEX(shipp));
	}

	//We're done!
	return LUA_RETURN_NIL;
}

SCRIPT_LUA_CALLBACK lua_msn_ship_getTarget(lua_State *L)
{
	ship *shipp = NULL;

	if(!lua_get_args(L, "o", l_ship.GetFromLua(&shipp)))
		return 0;

	if(shipp->ai_index != -1)
	{
		ai_info *aip= &Ai_info[shipp->ai_index];
		if(aip->target_objnum && Objects[aip->target_objnum].type == OBJ_SHIP)
		{
			ship *tshipp = &Ships[Objects[aip->target_objnum].instance];
			return lua_set_args(L, "o", l_ship.SetToLua(&tshipp));
		}
	}

	//We're done!
	return 0;
}

static const script_lua_obj_func_list lua_msn_shipinfo_funcs[] = {
	{"print",lua_msn_ShipInfo_print, "Prints ship name"},
	{SCRIPT_END_LIST},
};

static const script_lua_obj_func_list lua_msn_ship_funcs[] = {
	{"setSpeed", lua_msn_ship_setSpeed, "Sets ship speed: (x speed, y speed, z speed)"},
	{"setName", lua_msn_ship_setName, "Changes ship name: (new name)"},
	{"getTarget", lua_msn_ship_getTarget, "Returns handle to ship's targetted ship"},
	{SCRIPT_END_LIST},
};

//LIBRARY define
static const script_lua_obj_list lua_msn_lib_obj[] = {
	{l_ship.GetName(),		 lua_msn_ship_funcs, "Ship handle"},
	{l_ship_info.GetName(),	 lua_msn_shipinfo_funcs, "Ship info handle"},
	{SCRIPT_END_LIST},
};
static const script_lua_func_list lua_msn_lib[] = {
	{"newShip",			lua_msn_newShip, "Creates a new ship and returns a handle to it: (ship name, class name, x pos, y pos, z pos; x rot, y rot, z rot)"},
	{"getShipInfo",		lua_msn_getShipInfo, "Gets ship info object"},
	{SCRIPT_END_LIST},
};

// **********LIBRARY: Graphics (grl)
// *****Class: model
SCRIPT_LUA_CALLBACK lua_grpc_model_getFilename(lua_State *L)
{
	polymodel *pm;
	if(!lua_get_args(L, "o", l_model.GetFromLua(&pm)))
		return 0;

	return lua_set_args(L, "s", pm->filename);
}

SCRIPT_LUA_CALLBACK lua_grpc_model_getnumEyepoints(lua_State *L)
{
	polymodel *pm;
	if(!lua_get_args(L, "o", l_model.GetFromLua(&pm)))
		return 0;

	return lua_set_args(L, "s", pm->filename);
}

SCRIPT_LUA_CALLBACK lua_grpc_model_getEyepointPos(lua_State *L)
{
	polymodel *pm;
	int idx;
	if(!lua_get_args(L, "oi", l_model.GetFromLua(&pm), &idx))
		return 0;

	if(idx > -1 && idx < pm->n_view_positions)
	{
		return lua_set_args(L, "o", l_lvec.SetToLua(&pm->view_positions[idx].pnt));
	}

	return LUA_RETURN_NIL;
}

static const script_lua_obj_func_list lua_grpc_model_funcs[] = {
	{"getFilename",			lua_grpc_model_getFilename, "Gets the model's filename",
							NULL,"string"},
	{"getnumEyepoints",		lua_grpc_model_getnumEyepoints, "Gets number of eyepoints on ship",
							NULL,"number"},
	{"getEyepointPos",		lua_grpc_model_getEyepointPos, "Gets selected eye position",
							"number","lvec"},
	{SCRIPT_END_LIST},
};

// *****Functions: Graphics library
SCRIPT_LUA_CALLBACK lua_grpc_loadModel(lua_State *L)
{
	char *model_name;
	if(!lua_get_args(L, "s", &model_name))
		return 0;

	int idx = model_load(model_name, 0, NULL, 0);
	if(idx != -1)
	{
		polymodel *pm = model_get(idx);
		if(pm != NULL) {
			return lua_set_args(L, "o", l_model.SetToLua(&pm));
		}
	}

	return LUA_RETURN_NIL;
}
SCRIPT_LUA_CALLBACK lua_grpc_setColor(lua_State *L)
{
	if(!Gr_inited)
		return 0;

	int r,g,b,a=255;

	if(!lua_get_args(L, "iii|i", &r, &g, &b, &a))
		return 0;

	color ac;
	gr_init_alphacolor(&ac,r,g,b,a);
	gr_set_color_fast(&ac);

	return 0;
}

SCRIPT_LUA_CALLBACK lua_grpc_setFont(lua_State *L)
{
	if(!Gr_inited)
		return 0;

	int fn;

	if(!lua_get_args(L, "i", &fn))
		return 0;

	gr_set_font(fn);

	return 0;
}

SCRIPT_LUA_CALLBACK lua_grpc_drawPixel(lua_State *L)
{
	if(!Gr_inited)
		return 0;

	int x,y;
	bool r=true;

	if(!lua_get_args(L, "ii|b", &x, &y, &r))
		return 0;

	gr_pixel(x,y,r);

	return 0;
}

SCRIPT_LUA_CALLBACK lua_grpc_drawLine(lua_State *L)
{
	if(!Gr_inited)
		return 0;

	int x1,y1,x2,y2;
	bool r=true;

	if(!lua_get_args(L, "iiii|b", &x1, &y1, &x2, &y2, &r))
		return 0;

	gr_line(x1,y1,x2,y2,r);

	return 0;
}

SCRIPT_LUA_CALLBACK lua_grpc_drawGradientLine(lua_State *L)
{
	if(!Gr_inited)
		return 0;

	int x1,y1,x2,y2;
	bool r=true;

	if(!lua_get_args(L, "iiii|b", &x1, &y1, &x2, &y2, &r))
		return 0;

	gr_gradient(x1,y1,x2,y2,r);

	return 0;
}

SCRIPT_LUA_CALLBACK lua_grpc_drawCircle(lua_State *L)
{
	if(!Gr_inited)
		return 0;

	int x,y,ra;
	bool r=true;

	if(!lua_get_args(L, "iii|b", &x,&y,&ra,&r))
		return 0;

	gr_circle(x,y, ra, r);

	return 0;
}

SCRIPT_LUA_CALLBACK lua_grpc_drawCurve(lua_State *L)
{
	if(!Gr_inited)
		return 0;

	int x,y,ra,d;

	if(!lua_get_args(L, "iiii|b", &x,&y,&ra,&d))
		return 0;

	gr_curve(x,y,ra,d);

	return 0;
}

SCRIPT_LUA_CALLBACK lua_grpc_drawText(lua_State *L)
{
	if(!Gr_inited)
		return 0;

	int x,y;
	char *s;
	bool r=true;

	if(!lua_get_args(L, "iis|b", &x, &y, &s, &r))
		return 0;

	gr_string(x,y,s,r);

	return 0;
}

SCRIPT_LUA_CALLBACK lua_grpc_flashScreen(lua_State *L)
{
	if(!Gr_inited)
		return 0;

	int r,g,b;

	if(!lua_get_args(L, "iii", &r, &g, &b))
		return 0;

	gr_flash(r,g,b);

	return 0;
}

//LIBRARY define
static const script_lua_func_list lua_grpc_lib[] = {
	{"loadModel",		lua_grpc_loadModel, "Loads the given model and returns a handle to it",
						"string","model"},
	{"setColor", lua_grpc_setColor, "Sets the current drawing color: (red, green, blue; alpha)"},
	{"setFont", lua_grpc_setFont, "Sets the current drawing font: (font number)"},
	{"drawPixel", lua_grpc_drawPixel, "Draws a pixel using the current color: (x, y; resize)"},
	{"drawLine", lua_grpc_drawLine, "Draws a line using the current color: (x1, y1, x2, y2; resize)"},
	{"drawGradientLine", lua_grpc_drawGradientLine, "Draws a line using the current color that gradually fades out: (x1, y1, x2, y2; resize)"},
	{"drawCircle", lua_grpc_drawCircle, "Draws a circle using the current color: (x,y,radius; resize)"},
	{"drawCurve", lua_grpc_drawCurve, "Draws a curve using the current color: (x,y,radius,direction)"},
	{"drawText", lua_grpc_drawText, "Draws text using the current font and color: (x, y, text; resize)"},
	{"flashScreen", lua_grpc_flashScreen, "Flashes the screen with the specified color: (red,green,blue)"},
	{SCRIPT_END_LIST},
};

static const script_lua_obj_list lua_grpc_lib_obj[] = {
	{l_model.GetName(), lua_grpc_model_funcs, "Model"},
	{SCRIPT_END_LIST},
};

// **********LIBRARY Array
//Add an item in here to add an item to scripting.
//Elements: {library name, library function list, library object list}
const script_lua_lib_list Lua_libraries[] = {
	{"grpc", lua_grpc_lib, lua_grpc_lib_obj, "Graphics library"},
	{"misn", lua_msn_lib, lua_msn_lib_obj, "Mission library"},
	{NULL, lua_base_lib, lua_base_lib_obj, "Base FS2 library"},
	{SCRIPT_END_LIST},
};
*/
#endif //USE_LUA
// *************************Housekeeping*************************

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
	lua_func_hh *func;
	lua_func_hh *func_end;
	int i;	//used later

	//CHECK FOR BAD THINGS
#ifndef NDEBUG
	lua_func_hh *ofunc;
	//Like libraries/objects with identical names
	mprintf(("LUA: Performing object/library name validity check"));
	for(; lib < lib_end; lib++)
	{
		for(; obj < obj_end; obj++)
		{
			if(!stricmp(lib->Name, obj->Name))
				Error(LOCATION, "Lua library '%s' and object '%s' have the same name. Get a coder.", lib->Name, obj->Name);
		}
	}

	//Do double-object check
	for(; obj < obj_end; obj++)
	{
		libobj = obj;
		for(; libobj < obj_end; libobj++) {
			if(!stricmp(obj->Name, libobj->Name))
				Error(LOCATION, "Lua object '%s' and object '%s' have the same name. Get a coder.", obj->Name, libobj->Name);
		}

		//Check for duplicate functions within objects
		func = &obj->Functions[0];
		func_end = &obj->Functions[obj->Functions.size()];
		for(; func < func_end; func++) {
			ofunc = func;
			for(; ofunc < func_end; ofunc++) {
				if(!stricmp(func->Name, ofunc->Name))
					Error(LOCATION, "Function '%s' and function '%s' have the same name within object '%s'. Get a coder.", func->Name, ofunc->Name, obj->Name);
			}
		}
	}

	//Do lib-on-lib check
	for(; lib < lib_end; lib++)
	{
		libobj = lib;
		for(; libobj < lib_end; libobj++) {
			if(!stricmp(lib->Name, libobj->Name))
				Error(LOCATION, "Lua library '%s' and library '%s' have the same name. Get a coder.", lib->Name, libobj->Name);
		}

		//Check for duplicate functions within libs
		func = &lib->Functions[0];
		func_end = &lib->Functions[lib->Functions.size()];
		for(; func < func_end; func++)
		{
			ofunc = func;
			for(; ofunc < func_end; ofunc++) {
				if(!stricmp(func->Name, ofunc->Name))
					Error(LOCATION, "Function '%s' and function '%s' have the same name within library '%s'. Get a coder.", func->Name, ofunc->Name, lib->Name);
			}
		}
	}
#endif

	//INITIALIZE ALL LIBRARY FUNCTIONS
	mprintf(("LUA: Initializing library functions"));
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
				lua_pushstring(L, lib->Name);		//Add a string to the stack
				lua_pushvalue(L, -2);							//Push the table
				lua_settable(L, LUA_GLOBALSINDEX);				//Register the table with the new name
			}

			func = &lib->Functions[0];
			func_end = &lib->Functions[lib->Functions.size()];
			for(; func < func_end; func++)
			{
				//Add each function
				lua_pushstring(L, func->Name);				//Push the function's name onto the stack
				lua_pushcclosure(L, func->Function, 0);		//Push the function pointer onto the stack
				lua_settable(L, -3);						//Add it into the current lib table
			}
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
				Assert(func->Function != NULL);

				//Register the function with the name given as a global
				lua_register(L, func->Name, func->Function);
			}
		}

		//Handle objects and their methods in a library
	}

	lib = &lua_Objects[0];
	lib_end = &lua_Objects[lua_Objects.size()];
	for(; lib < lib_end; lib++)
	{
		if(!luaL_newmetatable(L, lib->Name))
		{
			Warning(LOCATION, "Couldn't create metatable for object '%s'", lib->Name);
			continue;
		}
		//Get the absolute position of the object metatable for later use
		int table_loc = lua_gettop(L);

		//***Add the functions into the metatables
		//Because both the [] operator and function list share the "__index"
		//entry in the metatable, we must check for both and give an error
		//to be safe
		bool index_oper_already = false;
		bool index_meth_already = false;

		func = &lib->Functions[0];
		func_end = &lib->Functions[lib->Functions.size()];

		//WMC - This is a bit odd. Basically, to handle derivatives, I have a double-loop set up.
		for(i = 0; i < 2; i++)
		{
			for(; func < func_end; func++)
			{
				//WMC - First, do normal functions
				if(!strnicmp(func->Name, "__", 2))
				{
					if(!stricmp(func->Name, "__index"))
					{
						if(!index_meth_already){
							index_oper_already = true;
						} else {
							Error(LOCATION, "Attempt to set both an indexing operator and methods for Lua class '%s'; get a coder", lib->Name);
						}
					}
					lua_pushstring(L, func->Name);
					lua_pushcclosure(L, func->Function, 0);
					lua_settable(L, table_loc);
				}
				else	//This is an object method
				{
					if(index_oper_already) {
						Error(LOCATION, "Attempt to set both an indexing operator and methods for Lua class '%s'; get a coder", lib->Name);
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
					lua_pushcclosure(L, func->Function, 0);
					lua_settable(L, -3);
				}
			}

			//WMC - Now set function pointers to the derivator (source class). And loop!
			//If there is no derivator, set them both to NULL to kill the loop like they did with Old Yeller
			if(lib->Derivator > -1) {
				func = &lua_Objects[lib->Derivator].Functions[0];
				func_end = &lua_Objects[lib->Derivator].Functions[lua_Objects[lib->Derivator].Functions.size()];
			} else {
				//Die Old Yeller, die!
				//func = func_end = NULL;
				//Damn, I can just do it this way.
				break;
			}
		}
	}
	SetLuaSession(L);

	return 1;
#else
	return 0;
#endif
}

#ifdef USE_LUA
void output_lib_meta(FILE *fp, lua_lib_h *lib, lua_lib_h *lib_deriv)
{
	lua_func_hh *func, *func_end;
	fputs("<dd><dl>", fp);
	for(int i = 0; i < 2; i++)
	{
		func = &lib->Functions[0];
		func_end = &lib->Functions[lib->Functions.size()];
		for(; func < func_end; func++)
		{
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
				fprintf(fp, "<dd><b>Return values:</b> %s</dd>", func->ReturnValues);
			} else {
				fputs("<dd><b>Return values:</b> None</dd>", fp);
			}
		}

		if(lib_deriv == NULL)
			break;

		lib = lib_deriv;
	}
	fputs("<br></dl></dd>", fp);
}
#endif

void script_state::OutputLuaMeta(FILE *fp)
{
#ifdef USE_LUA
	bool unnamed_display = false;

	//***Output Libraries
	fputs("<dl>", fp);
	fputs("<dt>Libraries</dt>", fp);
	lua_lib_h *lib = &lua_Libraries[0];
	lua_lib_h *lib_end = &lua_Libraries[lua_Libraries.size()];
	for(; lib < lib_end; lib++)
	{
		fprintf(fp, "<dd><a href=\"#%s\">%s</a> - %s</dd>", lib->Name, lib->Name, lib->Description);
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
		fprintf(fp, "<dt id=\"%s\"><h2>%s - %s</h2></dt>", lib->Name, lib->Name, lib->Description);

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
