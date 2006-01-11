#include "parse/scripting.h"
#ifdef USE_LUA
#include <string.h>
#include "parse/lua.h"
#include "graphics/2d.h"
#include "ship/ship.h"
#include "Ship/shipfx.h"
#include "io/mouse.h"
#include "gamesequence/gamesequence.h"
#include "globalincs/pstypes.h"

//*************************Lua funcs*************************
int lua_parse_args(lua_State *L, char *fmt, ...);
int lua_return_args(lua_State *L, char* fmt, ...);
int script_remove_lib(lua_State *L, char *name);

void lua_stackdump(lua_State *L);

//*************************Lua helpers*************************

//Function helper helper
//Stores data about functions
struct lua_func_hh
{
	char *Name;
	lua_CFunction Function;
	char *Description;
	char *Arguments;
	char *ReturnValues;
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

public:
	lua_lib_h(char *in_name, char *in_desc){Name = in_name; Description = in_desc;}
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

//Used with the args functions to check a function type
//You should really use LUA_CVAR though
struct script_lua_opdata
{
	char *meta;
	void **buf;

	script_lua_opdata(char* in_meta, void** in_buf){meta=in_meta; buf=in_buf;}
};

struct script_lua_odata
{
	char *meta;
	void *buf;
	int size;

	script_lua_odata(char *in_meta, void *in_buf, int in_size){meta=in_meta;buf=in_buf;size=in_size;}
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
	lua_obj_h(char *in_name, char *in_desc) {
		lua_Objects.push_back(lua_lib_h(in_name, in_desc));
		obj_idx = lua_Objects.size()-1;
	}

	void AddFunc(lua_func_hh *f){lua_Objects[obj_idx].Functions.push_back(*f);}
};

//Function helper class
//Lets us add functions via its constructor
class lua_func_h {
public:
	lua_func_h(char *name, lua_CFunction func, lua_lib &lib, char *desc, char *args, char *retvals) {
		lua_func_hh f;

		f.Name = name;
		f.Function = func;
		f.Description = desc;
		f.Arguments = args;
		f.ReturnValues = retvals;

		lib.AddFunc(&f);
	}

	lua_func_h(char *name, lua_CFunction func, lua_obj_h &obj, char *desc, char *args, char *retvals) {
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
	lua_obj(char*in_name, char*in_desc):lua_obj_h(in_name, in_desc){};

	//StoreType *Create(lua_State *L){StoreType *ptr = (StoreType*)LUA_NEW_OBJ(L, meta, StoreType); return ptr;}
	script_lua_odata Return(StoreType *obj){return script_lua_odata(lua_Objects[obj_idx].Name, obj, sizeof(StoreType));}
	script_lua_odata Parse(StoreType *ptr){return script_lua_odata(lua_Objects[obj_idx].Name, ptr, sizeof(StoreType));}
	script_lua_opdata ParsePtr(StoreType **ptr){return script_lua_opdata(lua_Objects[obj_idx].Name, ptr);}
};

//Function macro
//This is what you call to make new functions
#define LUA_FUNC(name, objlib, desc, args, retvals)	\
	static int lua_##objlib##_##name(lua_State *L);	\
	lua_func_h lua_##objlib##_##name##_h(#name, lua_##objlib##_##name, objlib, desc, args, retvals);	\
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
	script_lua_odata Parse(StoreType *ptr){return script_lua_odata(meta, ptr, sizeof(StoreType));}
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
//Call this with lua_parse_args to store the pointer to
//a specific set of userdata in a pointer.
//-----EX:
//lua_parse_args(L, "u", LUA_CVAR("ship", &lua_ship))
#define LUA_CVAR(n, p) script_lua_udata(n, (void**)&p)

//#define LUA_PTR(n, p) script_lua_pdata(n, (void*)p)
//*************************Lua return values*************************
#define LUA_RETURN_NOTHING		0
#define LUA_RETURN_OBJECT		1

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

//lua_parse_args(state, arguments, variables)
//----------------------------------------------
//based on "Programming in Lua"
//
//Parses arguments from string to variables given
//a '|' divides required and optional arguments.
//Returns 0 if a required argument is invalid,
//or there are too few arguments actually passed
int lua_parse_args(lua_State *L, char *fmt, ...)
{
	//Check that we have all the arguments that we need
	//If we don't, return 0
	int needed_args = strlen(fmt);
	int total_args = lua_gettop(L);
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
	nargs = 1;
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
				Error(LOCATION, "Bad character passed to lua_parse_args; (%c)", *(fmt-1));
				break;
		}
		nargs++;
	}
	va_end(vl);
	return nargs;
}
int lua_return_args(lua_State *L, char *fmt, ...)
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
				Error(LOCATION, "Bad character passed to lua_return_args; (%c)", *fmt);
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

//**********CLASS: Shipclass
lua_obj<int> l_Shipclass("shipclass", "Ship class object");
extern int ships_inited;

LUA_FUNC(getName, l_Shipclass, "Gets ship class name", NULL, "Name (string)")
{
	int idx;
	if(!lua_parse_args(L, "o", l_Shipclass.Parse(&idx)))
		return LUA_RETURN_NOTHING;

	if(idx < 0 || idx > Num_ship_classes)
		return LUA_RETURN_NOTHING;

	return lua_return_args(L, "s", Ship_info[idx].name);
}

LUA_FUNC(getShortName, l_Shipclass, "Gets ship class short name", NULL, "Short name (string)")
{
	int idx;
	if(!lua_parse_args(L, "o", l_Shipclass.Parse(&idx)))
		return LUA_RETURN_NOTHING;

	if(idx < 0 || idx > Num_ship_classes)
		return LUA_RETURN_NOTHING;

	return lua_return_args(L, "s", Ship_info[idx].short_name);
}

LUA_FUNC(getHitpoints, l_Shipclass, "Gets ship class hitpoints", NULL, "Hitpoints (number)")
{
	int idx;
	if(!lua_parse_args(L, "o", l_Shipclass.Parse(&idx)))
		return LUA_RETURN_NOTHING;

	if(idx < 0 || idx > Num_ship_classes)
		return LUA_RETURN_NOTHING;

	return lua_return_args(L, "f", Ship_info[idx].max_hull_strength);
}

//**********CLASS: Ship
lua_obj<int> l_Ship("ship", "Ship object");

//Helper function
int lua_ship_get_idx(lua_State *L, int *idx)
{
	if(!lua_parse_args(L, "o", l_Ship.Parse(idx)))
		return 0;

	*idx = ship_get_by_signature(*idx);

	if(*idx < 0)
		return 0;

	return 1;
}


LUA_FUNC(getName, l_Ship, "Gets ship name", NULL, "ship name (string)")
{
	int idx;
	if(!lua_ship_get_idx(L, &idx))
		return LUA_RETURN_NOTHING;

	return lua_return_args(L, "s", Ships[idx].ship_name);
}

LUA_FUNC(getHitpoints, l_Ship, "Gets ship hull max", NULL, "max hull hitpoints (number)")
{
	int idx;
	if(!lua_ship_get_idx(L, &idx))
		return LUA_RETURN_NOTHING;

	return lua_return_args(L, "f", Ship_info[Ships[idx].ship_info_index].max_hull_strength);
}

LUA_FUNC(getHitpointsLeft, l_Ship, "Gets ship hull", NULL, "Current hull hitpoints (number)")
{
	int idx;
	if(!lua_ship_get_idx(L, &idx))
		return LUA_RETURN_NOTHING;

	return lua_return_args(L, "f", Objects[Ships[idx].objnum].hull_strength);
}

LUA_FUNC(getClass, l_Ship, "Gets ship class handle", NULL, "Ship class handle (shipclass)")
{
	int idx;
	if(!lua_ship_get_idx(L, &idx))
		return LUA_RETURN_NOTHING;

	return lua_return_args(L, "o", l_Shipclass.Return(&Objects[Ships[idx].objnum].signature));
}

LUA_FUNC(warpIn, l_Ship, "Warps ship in", NULL, NULL)
{
	int idx;
	if(!lua_ship_get_idx(L, &idx))
		return LUA_RETURN_NOTHING;

	shipfx_warpin_start(&Objects[Ships[idx].objnum]);

	return LUA_RETURN_NOTHING;
}

LUA_FUNC(warpOut, l_Ship, "Warps ship out", NULL, NULL)
{
	int idx;
	if(!lua_ship_get_idx(L, &idx))
		return LUA_RETURN_NOTHING;

	shipfx_warpout_start(&Objects[Ships[idx].objnum]);

	return LUA_RETURN_NOTHING;
}

//**********LIBRARY: Mission
lua_lib l_Mission("mn", "Mission library");

LUA_FUNC(getShip, l_Mission, "Gets ship object", "Ship name", "Ship object")
{
	char *name;
	if(!lua_parse_args(L, "s", &name))
		return 0;

	int idx = ship_name_lookup(name);
	
	if(idx < 0) {
		return LUA_RETURN_NOTHING;
	}

	return lua_return_args(L, "o", l_Shipclass.Return(&Objects[Ships[idx].objnum].signature));
}

LUA_FUNC(getNumEscortShips, l_Mission, "Gets escort ship", NULL, "Ship object")
{
	return lua_return_args(L, "i", hud_escort_num_ships_on_list());
}

LUA_FUNC(getEscortShip, l_Mission, "Gets escort ship", "Escort index", "Ship object")
{
	int idx;
	if(!lua_parse_args(L, "i", &idx))
		return 0;

	if(idx < 0)
		return LUA_RETURN_NOTHING;

	idx = hud_escort_return_objnum(idx);
	
	if(idx < 0)
		return LUA_RETURN_NOTHING;

	return lua_return_args(L, "o", l_Ship.Return(&Objects[idx].signature));
}

//**********LIBRARY: Tables
lua_lib l_Tables("tb", "Tables library");

LUA_FUNC(getNumShipClasses, l_Tables, "Gets number of ship classes", NULL, "Number")
{
	if(!ships_inited)
		return lua_return_args(L, "i", 0);	//No ships loaded...should be 0

	return lua_return_args(L, "i", Num_ship_classes);
}

LUA_FUNC(getShipClassByIndex, l_Tables, "Gets ship class by index", "Class index", "Shipclass object")
{
	if(!ships_inited)
		return LUA_RETURN_NOTHING;

	int idx;
	if(!lua_parse_args(L, "i", &idx))
		return 0;
	
	if(idx < 0 || idx > Num_ship_classes) {
		return LUA_RETURN_NOTHING;
	}

	return lua_return_args(L, "o", l_Shipclass.Return(&idx));
}

LUA_FUNC(getShipClass, l_Tables, "Gets ship class", "Class name", "Shipclass object")
{
	if(!ships_inited)
		return LUA_RETURN_NOTHING;

	char *name;
	if(!lua_parse_args(L, "s", &name))
		return 0;

	int idx = ship_info_lookup(name);
	
	if(idx < 0) {
		return LUA_RETURN_NOTHING;
	}

	return lua_return_args(L, "o", l_Shipclass.Return(&idx));
}

//**********LIBRARY: Mouse
lua_lib l_Mouse("ms", "Mouse library");

extern int mouse_inited;

LUA_FUNC(getX, l_Mouse, "Gets Mouse X pos", "[Unscale]", "X pos (Number)")
{
	if(!mouse_inited)
		return LUA_RETURN_NOTHING;

	int x;
	bool u = false;
	lua_parse_args(L, "b", &u);

	if(u)
		mouse_get_pos_unscaled(&x, NULL);
	else
		mouse_get_pos(&x, NULL);

	return lua_return_args(L, "i", x);
}

LUA_FUNC(getY, l_Mouse, "Gets Mouse Y pos", "[Unscale]", "Y pos (Number)")
{
	if(!mouse_inited)
		return LUA_RETURN_NOTHING;

	int y;
	bool u = false;
	lua_parse_args(L, "b", &u);

	if(u)
		mouse_get_pos_unscaled(NULL, &y);
	else
		mouse_get_pos(NULL, &y);

	return lua_return_args(L, "i", y);
}

LUA_FUNC(isButtonDown, l_Mouse, "Returns whether the specified mouse buttons are up or down", "{Left, Right, or Middle}, [...], [...]", "Whether specified buttons are pressed (Boolean)")
{
	if(!mouse_inited)
		return LUA_RETURN_NOTHING;

	char *sa[3] = {NULL, NULL, NULL};
	lua_parse_args(L, "s|ss", &sa[0], &sa[1], &sa[2]);	//Like a snake!

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

	return lua_return_args(L, "b", rtn);
}

LUA_FUNC(setCursorImage, l_Mouse, "Sets mouse cursor image, and allows you to lock/unlock the image. (A locked cursor may only be changed with the unlock parameter)", "Image filename w/o extension, [Lock,Unlock]", "Y pos (Number)")
{
	if(!mouse_inited || !Gr_inited)
		return LUA_RETURN_NOTHING;

	char *s = NULL;
	char *u = NULL;
	if(!lua_parse_args(L, "s|s", &s, &u))
		return LUA_RETURN_NOTHING;

	int ul = 0;
	if(u != NULL)
	{
		if(!stricmp("Lock", u))
			ul = GR_CURSOR_LOCK;
		else if(!stricmp("Unlock", u))
			ul = GR_CURSOR_UNLOCK;
	}

	gr_set_cursor_bitmap(bm_load(s), ul);

	return LUA_RETURN_NOTHING;
}

LUA_FUNC(setMouseHidden, l_Mouse, "Shows or hides mouse cursor", "True to hide mouse, false to show it", NULL)
{
	if(!mouse_inited)
		return LUA_RETURN_NOTHING;

	bool b = false;
	lua_parse_args(L, "b", &b);

	if(b)
		Mouse_hidden = 1;
	else
		Mouse_hidden = 0;

	return LUA_RETURN_NOTHING;
}

//**********LIBRARY: Graphics
lua_lib l_Graphics("gr", "Graphics Library");

LUA_FUNC(getScreenWidth, l_Graphics, "Gets screen width", NULL, "Width in pixels (Number)")
{
	if(!Gr_inited)
		return 0;

	return lua_return_args(L, "i", gr_screen.max_w);
}

LUA_FUNC(getScreenHeight, l_Graphics, "Gets screen height", NULL, "Height in pixels (Number)")
{
	if(!Gr_inited)
		return 0;

	return lua_return_args(L, "i", gr_screen.max_h);
}

LUA_FUNC(drawText, l_Graphics, "Draws a string", "x, y, String, [Resize]", NULL)
{
	if(!Gr_inited)
		return 0;

	int x,y;
	char *s;
	bool r=true;

	if(!lua_parse_args(L, "iis|b", &x, &y, &s, &r))
		return LUA_RETURN_NOTHING;

	gr_string(x,y,s,r);

	return LUA_RETURN_NOTHING;
}

LUA_FUNC(setColor, l_Graphics, "Sets 2D drawing color", "Red, Green, Blue, [alpha]", NULL)
{
	if(!Gr_inited)
		return 0;

	int r,g,b,a=255;

	if(!lua_parse_args(L, "iii|i", &r, &g, &b, &a))
		return 0;

	color ac;
	gr_init_alphacolor(&ac,r,g,b,a);
	gr_set_color_fast(&ac);

	return 0;
}

LUA_FUNC(setFont, l_Graphics, "Sets current font", "Font index", NULL)
{
	if(!Gr_inited)
		return 0;

	int fn;

	if(!lua_parse_args(L, "i", &fn))
		return 0;

	gr_set_font(fn);

	return 0;
}

LUA_FUNC(drawPixel, l_Graphics, "Sets pixel to current color", "x, y, [Resize]", NULL)
{
	if(!Gr_inited)
		return 0;

	int x,y;
	bool r=true;

	if(!lua_parse_args(L, "ii|b", &x, &y, &r))
		return 0;

	gr_pixel(x,y,r);

	return 0;
}

LUA_FUNC(drawLine, l_Graphics, "Draws a line with the current color", "x1, y1, x2, y2, [Resize]", NULL)
{
	if(!Gr_inited)
		return 0;

	int x1,y1,x2,y2;
	bool r=true;

	if(!lua_parse_args(L, "iiii|b", &x1, &y1, &x2, &y2, &r))
		return 0;

	gr_line(x1,y1,x2,y2,r);

	return 0;
}

LUA_FUNC(drawGradientLine, l_Graphics, "Draws a line that steadily fades out", "x1, y1, x2, y2, [Resize]", NULL)
{
	if(!Gr_inited)
		return 0;

	int x1,y1,x2,y2;
	bool r=true;

	if(!lua_parse_args(L, "iiii|b", &x1, &y1, &x2, &y2, &r))
		return 0;

	gr_gradient(x1,y1,x2,y2,r);

	return 0;
}

LUA_FUNC(drawCircle, l_Graphics, "Draws a circle", "x, y, Radius, [Resize]", NULL)
{
	if(!Gr_inited)
		return 0;

	int x,y,ra;
	bool r=true;

	if(!lua_parse_args(L, "iii|b", &x,&y,&ra,&r))
		return 0;

	gr_circle(x,y, ra, r);

	return 0;
}

LUA_FUNC(drawCurve, l_Graphics, "Draws a curve", "x, y, Radius, Direction", NULL)
{
	if(!Gr_inited)
		return 0;

	int x,y,ra,d;

	if(!lua_parse_args(L, "iiii|b", &x,&y,&ra,&d))
		return 0;

	gr_curve(x,y,ra,d);

	return 0;
}

LUA_FUNC(drawImage, l_Graphics, "Draws an image", "x, y, Image name, [Resize]", NULL)
{
	if(!Gr_inited)
		return 0;

	int x,y;
	char *s;
	bool r = true;

	if(!lua_parse_args(L, "iis|b", &x,&y,&s,&r))
		return 0;

	int idx = bm_load(s);
	gr_set_bitmap(idx);
	gr_bitmap(x, y, r);

	return 0;
}

LUA_FUNC(flashScreen, l_Graphics, "Flashes the screen", "Red, Green, Blue", NULL)
{
	if(!Gr_inited)
		return 0;

	int r,g,b;

	if(!lua_parse_args(L, "iii", &r, &g, &b))
		return 0;

	gr_flash(r,g,b);

	return 0;
}

//*************************Libraries*************************
/*
//**********LIBRARY: Base
//*****CLASS: (l)vec
SCRIPT_LUA_CALLBACK lua_fs2_lvec(lua_State *L)
{
	vec3d v3 = vmd_zero_vector;
	lua_parse_args(L, "|fff", &v3.xyz.x, &v3.xyz.y, &v3.xyz.z);

	return lua_return_args(L, "o", l_lvec.Return(&v3));
}

SCRIPT_LUA_CALLBACK lua_fs2_vec__addition(lua_State *L)
{
	vec3d v3a, v3b, v3r;
	if(!lua_parse_args(L, "oo", l_lvec.Parse(&v3a), l_lvec.Parse(&v3b)))
		return 0;

	vm_vec_add(&v3r, &v3a, &v3b);

	return lua_return_args(L, "o", l_lvec.Return(&v3r));
}

SCRIPT_LUA_CALLBACK lua_fs2_vec__subtraction(lua_State *L)
{
	vec3d v3a, v3b, v3r;
	if(!lua_parse_args(L, "oo", l_lvec.Parse(&v3a), l_lvec.Parse(&v3b)))
		return 0;

	vm_vec_sub(&v3r, &v3a, &v3b);

	return lua_return_args(L, "o", l_lvec.Return(&v3r));
}

SCRIPT_LUA_CALLBACK lua_fs2_vec__multiplication(lua_State *L)
{
	vec3d v3a, v3b, v3r;
	float f;
	if(lua_parse_args(L, "of", l_lvec.Parse(&v3a), &f))
	{
		vm_vec_copy_scale(&v3r, &v3a, f);

		return lua_return_args(L, "o", l_lvec.Return(&v3r));
	}
	else if(lua_parse_args(L, "oo", l_lvec.Parse(&v3a), l_lvec.Parse(&v3b)))
	{
		v3r.xyz.x = v3a.xyz.x * v3b.xyz.x;
		v3r.xyz.y = v3a.xyz.y * v3b.xyz.y;
		v3r.xyz.z = v3a.xyz.z * v3b.xyz.z;

		return lua_return_args(L, "o", l_lvec.Return(&v3r));
	}

	//Neither is valid...

	return LUA_RETURN_NOTHING;
}


SCRIPT_LUA_CALLBACK lua_fs2_vec__tostring(lua_State *L)
{
	vec3d v3p;
	if(!lua_parse_args(L, "o", l_lvec.Parse(&v3p)))
		return 0;

	char buf[32];
	sprintf(buf, "(%f, %f, %f)", v3p.xyz.x, v3p.xyz.y, v3p.xyz.z);

	return lua_return_args(L, "s", buf);
}

SCRIPT_LUA_CALLBACK lua_fs2_vec__index(lua_State *L)
{
	vec3d v3p;
	int idx=0;
	if(!lua_parse_args(L, "o|i", l_lvec.Parse(&v3p), &idx))
		return 0;

	if(idx < 0 || idx > 2) {
		return 0;
	}

	return lua_return_args(L, "f", v3p.a1d[idx]);
}

SCRIPT_LUA_CALLBACK lua_fs2_vec_print(lua_State *L)
{
	vec3d v3p;
	int idx=0;
	if(!lua_parse_args(L, "o|i", l_lvec.Parse(&v3p), &idx))
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

//*****CLASS: wvec
SCRIPT_LUA_CALLBACK lua_fs2_wvec(lua_State *L)
{
	vec3d v3 = vmd_zero_vector;
	lua_parse_args(L, "|fff", &v3.xyz.x, &v3.xyz.y, &v3.xyz.z);

	vec3d v3p;
	v3p.xyz.x = v3.xyz.x;
	v3p.xyz.y = v3.xyz.y;
	v3p.xyz.z = v3.xyz.z;

	return lua_return_args(L, "o", l_wvec.Return(&v3p));
}

static const script_lua_obj_func_list lua_fs2_wvec_funcs[] = {
	{LUA_OPER_ADDITION, lua_fs2_vec__addition, "Adds two vectors"},
	{LUA_OPER_SUBTRACTION, lua_fs2_vec__subtraction, "Subtracts one vector from another"},
	{LUA_OPER_MULTIPLICATION, lua_fs2_vec__multiplication, "Multiplies two vectors, or a vector times a number"},
	{LUA_OPER_TOSTRING, lua_fs2_vec__tostring, "Converts vector to string"},
	//{LUA_OPER_INDEX, lua_fs2_vec__index, "Returns index into vector"},
	{SCRIPT_END_LIST},
};

//*****LIBRARY CLASSES

static const script_lua_obj_list lua_base_lib_obj[] = {
	{l_lvec.GetName(), lua_fs2_lvec_funcs, "Local vector"},
	{l_wvec.GetName(), lua_fs2_wvec_funcs, "World vector"},
	{SCRIPT_END_LIST},
};

//*****LIBRARY FUNCTIONS

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
	lua_parse_args(L, "|i", &depth);

	return lua_return_args(L, "s", GS_state_text[gameseq_get_state(depth)]);
}

static const script_lua_func_list lua_base_lib[] = {
	{"lvec", lua_fs2_lvec, "Creates a new local vector: (x, y, z)"},
	{"wvec", lua_fs2_wvec, "Creates a new world vector: (x, y, z)"},
	{"print", lua_fs2_print, "Displays output"},
	{"error", lua_fs2_error, "Causes an error"},
	{"getState", lua_fs2_getState, "Returns the current FS2 state string"},
	{SCRIPT_END_LIST},
};

//**********LIBRARY: Mission (msn)

SCRIPT_LUA_CALLBACK lua_msn_getShipInfo(lua_State *L)
{
	char *ship_name = NULL;
	if(!lua_parse_args(L, "s", &ship_name))
		return 0;

	int idx = ship_info_lookup(ship_name);

	if(idx < 0)
		return 0;

	ship_info *sip = &Ship_info[idx];

	return lua_return_args(L, "o", l_ship_info.Return(&sip));
}

SCRIPT_LUA_CALLBACK lua_msn_ShipInfo_print(lua_State *L)
{
	ship_info *sip;
	if(!lua_parse_args(L, "o", l_ship_info.Parse(&sip)))
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

	if(!lua_parse_args(L, "ssfff|fff", &ship_name, &class_name, &pos.xyz.x, &pos.xyz.y, &pos.xyz.z, 
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
		return LUA_RETURN_NOTHING;
	}

	//Create the ship
	int s_idx = ship_create(&ori, &pos, si_idx, ship_name);

	//Make the lua ship object, if the ship was created
	if(s_idx > -1)
	{
		//Make the lua ship object and set the ptr to NULL
		ship *shipp = &Ships[s_idx];
		return lua_return_args(L, "o", l_ship.Return(&shipp));
	}

	return LUA_RETURN_NOTHING;
}

SCRIPT_LUA_CALLBACK lua_msn_ship_setSpeed(lua_State *L)
{
	ship *shipp = NULL;
	vec3d vel;

	if(!lua_parse_args(L, "offf", l_ship.Parse(&shipp), &vel.xyz.x, &vel.xyz.y, &vel.xyz.z))
		return LUA_RETURN_NOTHING;

	//Set the speed
	Objects[shipp->objnum].phys_info.vel = vel;

	//We're done!
	return LUA_RETURN_NOTHING;
}

SCRIPT_LUA_CALLBACK lua_msn_ship_setName(lua_State *L)
{
	ship *shipp = NULL;
	char *name = NULL;

	if(!lua_parse_args(L, "os", l_ship.Parse(&shipp), &name))
		return LUA_RETURN_NOTHING;

	//Set the name
	if(strlen(name)) {
		strcpy(shipp->ship_name, name);
	} else {
		sprintf(shipp->ship_name, "Ship %d", SHIP_INDEX(shipp));
	}

	//We're done!
	return LUA_RETURN_NOTHING;
}

SCRIPT_LUA_CALLBACK lua_msn_ship_getTarget(lua_State *L)
{
	ship *shipp = NULL;

	if(!lua_parse_args(L, "o", l_ship.Parse(&shipp)))
		return 0;

	if(shipp->ai_index != -1)
	{
		ai_info *aip= &Ai_info[shipp->ai_index];
		if(aip->target_objnum && Objects[aip->target_objnum].type == OBJ_SHIP)
		{
			ship *tshipp = &Ships[Objects[aip->target_objnum].instance];
			return lua_return_args(L, "o", l_ship.Return(&tshipp));
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

//**********LIBRARY: Graphics (grl)
//*****Class: model
SCRIPT_LUA_CALLBACK lua_grpc_model_getFilename(lua_State *L)
{
	polymodel *pm;
	if(!lua_parse_args(L, "o", l_model.Parse(&pm)))
		return 0;

	return lua_return_args(L, "s", pm->filename);
}

SCRIPT_LUA_CALLBACK lua_grpc_model_getnumEyepoints(lua_State *L)
{
	polymodel *pm;
	if(!lua_parse_args(L, "o", l_model.Parse(&pm)))
		return 0;

	return lua_return_args(L, "s", pm->filename);
}

SCRIPT_LUA_CALLBACK lua_grpc_model_getEyepointPos(lua_State *L)
{
	polymodel *pm;
	int idx;
	if(!lua_parse_args(L, "oi", l_model.Parse(&pm), &idx))
		return 0;

	if(idx > -1 && idx < pm->n_view_positions)
	{
		return lua_return_args(L, "o", l_lvec.Return(&pm->view_positions[idx].pnt));
	}

	return LUA_RETURN_NOTHING;
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

//*****Functions: Graphics library
SCRIPT_LUA_CALLBACK lua_grpc_loadModel(lua_State *L)
{
	char *model_name;
	if(!lua_parse_args(L, "s", &model_name))
		return 0;

	int idx = model_load(model_name, 0, NULL, 0);
	if(idx != -1)
	{
		polymodel *pm = model_get(idx);
		if(pm != NULL) {
			return lua_return_args(L, "o", l_model.Return(&pm));
		}
	}

	return LUA_RETURN_NOTHING;
}
SCRIPT_LUA_CALLBACK lua_grpc_setColor(lua_State *L)
{
	if(!Gr_inited)
		return 0;

	int r,g,b,a=255;

	if(!lua_parse_args(L, "iii|i", &r, &g, &b, &a))
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

	if(!lua_parse_args(L, "i", &fn))
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

	if(!lua_parse_args(L, "ii|b", &x, &y, &r))
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

	if(!lua_parse_args(L, "iiii|b", &x1, &y1, &x2, &y2, &r))
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

	if(!lua_parse_args(L, "iiii|b", &x1, &y1, &x2, &y2, &r))
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

	if(!lua_parse_args(L, "iii|b", &x,&y,&ra,&r))
		return 0;

	gr_circle(x,y, ra, r);

	return 0;
}

SCRIPT_LUA_CALLBACK lua_grpc_drawCurve(lua_State *L)
{
	if(!Gr_inited)
		return 0;

	int x,y,ra,d;

	if(!lua_parse_args(L, "iiii|b", &x,&y,&ra,&d))
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

	if(!lua_parse_args(L, "iis|b", &x, &y, &s, &r))
		return 0;

	gr_string(x,y,s,r);

	return 0;
}

SCRIPT_LUA_CALLBACK lua_grpc_flashScreen(lua_State *L)
{
	if(!Gr_inited)
		return 0;

	int r,g,b;

	if(!lua_parse_args(L, "iii", &r, &g, &b))
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

//**********LIBRARY Array
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
//*************************Housekeeping*************************

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

	//INITIALIZE ALL LIBRARY FUNCTIONS
	lua_lib_h *lib = &lua_Libraries[0];
	lua_lib_h *lib_end = &lua_Libraries[lua_Libraries.size()];
	lua_func_hh *func;
	lua_func_hh *func_end;

	for(lib = &lua_Libraries[0]; lib < lib_end; lib++)
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
		for(; func < func_end; func++)
		{
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
	}
	SetLuaSession(L);

	return 1;
#else
	return 0;
#endif
}

#ifdef USE_LUA
void output_lib_meta(FILE *fp, lua_lib_h *lib)
{
	lua_func_hh *func = &lib->Functions[0];
	lua_func_hh *func_end = &lib->Functions[lib->Functions.size()];

	fputs("<dd><dl>", fp);
	for(; func < func_end; func++)
	{
		if(func->Arguments != NULL) {
			fprintf(fp, "<dt><b>%s(</b><i>%s</i><b>)</b></dt>", func->Name, func->Arguments);
		} else {
			fprintf(fp, "<dt><b>%sNone</b></dt>", func->Name);
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

		output_lib_meta(fp, lib);
	}
	//***Output objects
	lib = &lua_Objects[0];
	lib_end = &lua_Objects[lua_Objects.size()];
	for(; lib < lib_end; lib++)
	{
		fprintf(fp, "<dt id=\"%s\"><h2>%s - %s</h2></dt>", lib->Name, lib->Name, lib->Description);

		output_lib_meta(fp, lib);
	}
	fputs("</dl>", fp);
#endif
}