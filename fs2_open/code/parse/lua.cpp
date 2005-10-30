#include "parse/scripting.h"
#ifdef USE_LUA
#include <string.h>
#include "parse/lua.h"
#include "graphics/2d.h"
#include "ship/ship.h"
#include "gamesequence/gamesequence.h"
#include "globalincs/pstypes.h"

//*************************Lua funcs*************************
int script_parse_args(lua_State *L, char *fmt, ...);
int script_return_args(lua_State *L, char* fmt, ...);
int script_remove_lib(lua_State *L, char *name);

void lua_stackdump(lua_State *L);

//*************************Lua helpers*************************
typedef struct script_lua_func_list {
	const char *name;
	lua_CFunction func;
	const char *description;
} script_lua_func_list;

typedef struct script_lua_obj_func_list {
	const char *name;
	lua_CFunction func;
	int type;
	const char *description;
} script_lua_obj_func_list;

typedef struct script_lua_obj_list
{
	const char *object_name;
	const script_lua_obj_func_list *object_funcs;
	const char *description;
}script_lua_obj_list;

typedef struct script_lua_lib_list
{
	const char *library_name;
	const script_lua_func_list *library_funcs;
	const script_lua_obj_list *library_objects;
	const char *description;
}script_lua_lib_list;

//Used with the args functions to check a function type
struct script_lua_udata
{
	char *meta;
	void **buf;

	script_lua_udata(char* in_meta, void** in_buf){meta=in_meta; buf=in_buf;}
};

//LUA_UDATA(name, pointer)
//Call this with script_parse_args to store the pointer to
//a specific set of userdata in a pointer.
//-----EX:
//script_parse_args(L, "u", LUA_UDATA("ship", &lua_ship))
#define LUA_UDATA(n, p) script_lua_udata(n, (void**)&p)

#define LUA_RETURN_OBJECT	1

//LUA_NEW_OBJ(lua_State, object name, object struct)
#define LUA_NEW_OBJ(L,n,s)					\
	(s *) lua_newuserdata(L, sizeof(s));	\
	luaL_getmetatable(L, n);				\
	lua_setmetatable(L, -2)

//*************************Lua defines*************************
enum {
	LUA_OPER_METHOD		=0,
	LUA_OPER_ADDITION,
	LUA_OPER_SUBTRACTION,
	LUA_OPER_MULTIPLICATION,
	LUA_OPER_DIVISION,
	LUA_OPER_POWER,
	LUA_OPER_UNARY,
	LUA_OPER_CONCATENATION,
	LUA_OPER_EQUALTO,
	LUA_OPER_LESSTHAN,
	LUA_OPER_LESSTHANEQUALTO,
	LUA_OPER_INDEX,
	LUA_OPER_NEWINDEX,
	LUA_OPER_CALL
};

char *Lua_opers[] = {
	"__index",
	"__add",
	"__sub",
	"__mult",
	"__div",
	"__pow",
	"__unm",
	"__concat",
	"__eq",
	"__lt",
	"__le",
	"__index",
	"__newindex",
	"__call",
};

//*************************Functions*************************
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

void lua_stackdump(lua_State *L)
{
	char stackdump[10240] = "\0";
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
		switch(type)
		{
			case LUA_TNIL:
				strcat(stackdump, "\r\nNIL");
				break;
			case LUA_TNUMBER:
				d = lua_tonumber(L, argnum);
				sprintf(buf, "\r\nNumber [%f]",d);
				strcat(stackdump, buf);
				break;
			case LUA_TBOOLEAN:
				b = lua_toboolean(L, argnum);
				sprintf(buf, "\r\nBoolean [%d]",b);
				strcat(stackdump, buf);
				break;
			case LUA_TSTRING:
				s = (char *)lua_tostring(L, argnum);
				sprintf(buf, "\r\nString [%s]",s);
				strcat(stackdump, buf);
				break;
			case LUA_TTABLE:
				strcat(stackdump, "\r\nTable");
				break;
			case LUA_TFUNCTION:
				strcat(stackdump, "\r\nFunction");
				break;
			case LUA_TUSERDATA:
				v = lua_touserdata(L, argnum);
				sprintf(buf, "\r\nUserdata [%d]", v);
				strcat(stackdump, buf);
				break;
			case LUA_TTHREAD:
				ls = lua_tothread(L, argnum);
				sprintf(buf, "\r\nThread [%d]", ls);
				strcat(stackdump, buf);
				break;
			case LUA_TLIGHTUSERDATA:
				v = lua_touserdata(L, argnum);
				sprintf(buf, "\r\nLight userdata [%d]", v);
				strcat(stackdump, buf);
				break;
			default:
				sprintf(buf, "\r\n<UNKNOWN>: %s (%d) (%s)", lua_typename(L, type), lua_tonumber(L, argnum), lua_tostring(L, argnum));
				strcat(stackdump, buf);
				break;
		}
	}

	Warning(LOCATION, "LUA STACKDUMP:%s",stackdump);
}

int script_return_args(lua_State *L, char *fmt, ...)
{
	//Start throught
	va_list vl;
	int nargs;

	va_start(vl, fmt);
	nargs = 1;
	while(*fmt)
	{
		switch(*fmt++)
		{
			case 'b':
				lua_pushboolean(L, va_arg(vl, double) ? 1 : 0);
				break;
			case 'd':
				lua_pushnumber(L, va_arg(vl, double));
				break;
			case 'f':
				lua_pushnumber(L, va_arg(vl, float));
				break;
			case 'i':
				lua_pushnumber(L, va_arg(vl, int));
				break;
			case 's':
				lua_pushstring(L, va_arg(vl, char *));
				break;
			default:
				Error(LOCATION, "Bad character passed to script_return_args; (%c)", *fmt);
		}
		nargs++;
	}
	va_end(vl);
	return nargs;
}

//script_parse_args(state, arguments, variables)
//----------------------------------------------
//based on "Programming in Lua"
//
//Parses arguments from string to variables given
//a '|' divides required and optional arguments.
//Returns 0 if a required argument is invalid,
//or there are too few arguments actually passed
int script_parse_args(lua_State *L, char *fmt, ...)
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
		lua_stackdump(L);
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
					Warning(LOCATION, "Argument %d is an invalid type; boolean expected");
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
					lua_stackdump(L);
					if(!optional_args) return 0;
				}
				break;
			case 'u':
				if(lua_isuserdata(L, nargs))
				{
					script_lua_udata ud = va_arg(vl, script_lua_udata);
					(*ud.buf) = luaL_checkudata(L, nargs, ud.meta);
					if((*ud.buf) == NULL) {
						Warning(LOCATION, "Argument %d is the wrong type of userdata; %s expected", nargs, ud.meta);
					}
				}
				else
				{
					Warning(LOCATION, "Argument %d is an invalid type %d; type '%s' expected", nargs, lua_type(L, nargs), va_arg(vl, script_lua_udata).meta);
					if(!optional_args) return 0;
				}
				break;
			case '|':
				nargs--;	//cancel out the nargs++ at the end
				optional_args = true;
				break;
			default:
				Error(LOCATION, "Bad character passed to script_parse_args; (%c)", *fmt);
		}
		nargs++;
	}
	va_end(vl);
	return nargs;
}

//*************************Libraries*************************

//**********LIBRARY: Base
static int lua_fs2_print(lua_State *L)
{
	Error(LOCATION, "LUA: %s", lua_tostring(L, -1));

	return 0;
}

static int lua_fs2_error(lua_State *L)
{
	Error(LOCATION, "LUA ERROR: %s", lua_tostring(L, -1));

	return 0;
}

static int lua_fs2_getState(lua_State *L)
{
	int depth = 0;
	script_parse_args(L, "|i", &depth);

	return script_return_args(L, "s", GS_state_text[gameseq_get_state(depth)]);
}

static int lua_fs2_lvec(lua_State *L)
{
	vec3d v3 = vmd_zero_vector;
	script_parse_args(L, "|fff", &v3.xyz.x, &v3.xyz.y, &v3.xyz.z);

	vec3d* v3p = LUA_NEW_OBJ(L, "fs2.lvec", vec3d);
	v3p->xyz.x = v3.xyz.x;
	v3p->xyz.y = v3.xyz.y;
	v3p->xyz.z = v3.xyz.z;

	return LUA_RETURN_OBJECT;
}

static int lua_fs2_lvec_print(lua_State *L)
{
	vec3d *v3p;
	if(!script_parse_args(L, "u", LUA_UDATA("fs2.lvec", v3p)))
		return 0;

	Error(LOCATION, "VECTOR: %f %f %f", v3p->xyz.x, v3p->xyz.y, v3p->xyz.z);
	return 0;
}

static int lua_fs2_lvec_get(lua_State *L)
{
	vec3d *v3p;
	int idx;
	if(!script_parse_args(L, "ui", LUA_UDATA("fs2.lvec", v3p), &idx))
		return 0;

	if(idx < 3 && idx > -1)
	{
		Error(LOCATION, "VECTOR: %f", v3p->a1d[idx]);
	}
	return 0;
}

static int lua_fs2_wvec(lua_State *L)
{
	vec3d v3 = vmd_zero_vector;
	script_parse_args(L, "|fff", &v3.xyz.x, &v3.xyz.y, &v3.xyz.z);

	vec3d* v3p = LUA_NEW_OBJ(L, "fs2.wvec", vec3d);
	v3p->xyz.x = v3.xyz.x;
	v3p->xyz.y = v3.xyz.y;
	v3p->xyz.z = v3.xyz.z;

	return LUA_RETURN_OBJECT;
}

static const script_lua_obj_func_list lua_fs2_wvec_funcs[] = {
	{SCRIPT_END_LIST},
};

static const script_lua_obj_func_list lua_fs2_lvec_funcs[] = {
	{"print", lua_fs2_lvec_print, LUA_OPER_METHOD, "Prints contents of vector"},
	//{"get", lua_fs2_lvec_get, LUA_OPER_INDEX},
	{SCRIPT_END_LIST},
};

static const script_lua_obj_list lua_base_lib_obj[] = {
	{"fs2.lvec", lua_fs2_lvec_funcs, "Local vector"},
	{"fs2.wvec", lua_fs2_wvec_funcs, "World vector"},
	{SCRIPT_END_LIST},
};

static const script_lua_func_list lua_base_lib[] = {
	{"lvec", lua_fs2_lvec, "Creates a new local vector: (x, y, z)"},
	{"wvec", lua_fs2_wvec, "Creates a new world vector: (x, y, z)"},
	{"print", lua_fs2_print, "Displays output"},
	{"error", lua_fs2_error, "Causes an error"},
	{"getState", lua_fs2_getState, "Returns the current FS2 state string"},
	{SCRIPT_END_LIST},
};

//**********LIBRARY: Mission (msn)
typedef struct lua_ship
{
	ship *shipp;
} lua_ship;

typedef struct lua_ship_info
{
	ship_info *sip;
} lua_ship_info;

static int lua_msn_getShipInfo(lua_State *L)
{
	char *ship_name = NULL;
	if(!script_parse_args(L, "s", &ship_name))
		return 0;

	int idx = ship_info_lookup(ship_name);

	if(idx < 0)
		return 0;

	lua_ship_info *lsip = (lua_ship_info*) lua_newuserdata(L, sizeof(lua_ship_info));
	luaL_getmetatable(L, "ship_info");
	lua_setmetatable(L, -2);
	lsip->sip = &Ship_info[idx];

	return 1;
}

static int lua_msn_newShip(lua_State *L)
{
	char *ship_name = NULL;
	char *class_name = NULL;
	vec3d pos;
	angles ang = {0,0,0};

	if(!script_parse_args(L, "ssfff|fff", &ship_name, &class_name, &pos.xyz.x, &pos.xyz.y, &pos.xyz.z, 
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
		return 0;
	}

	//Create the ship
	int s_idx = ship_create(&ori, &pos, si_idx, ship_name);

	//Make the lua ship object, if the ship was created
	if(s_idx > -1)
	{
		//Make the lua ship object and set the ptr to NULL
		lua_ship *lsp = LUA_NEW_OBJ(L, "fs2.ship", lua_ship);
		lsp->shipp = &Ships[s_idx];
		return 1;
	}

	return 0;
}

static lua_msn_ship_setSpeed(lua_State *L)
{
	lua_ship *lsp = NULL;
	vec3d vel;

	if(!script_parse_args(L, "ufff", LUA_UDATA("fs2.ship", lsp), &vel.xyz.x, &vel.xyz.y, &vel.xyz.z))
		return 0;

	//Set the speed
	Objects[lsp->shipp->objnum].phys_info.vel = vel;

	//We're done!
	return 0;
}

static lua_msn_ship_setName(lua_State *L)
{
	lua_ship *lsp = NULL;
	char *name = NULL;

	if(!script_parse_args(L, "us", LUA_UDATA("fs2.ship", lsp), &name))
		return 0;

	//Set the name
	if(strlen(name)) {
		strcpy(lsp->shipp->ship_name, name);
	} else {
		sprintf(lsp->shipp->ship_name, "Ship %d", SHIP_INDEX(lsp->shipp));
	}

	//We're done!
	return 0;
}

static lua_msn_ship_getTarget(lua_State *L)
{
	lua_ship *lsp = NULL;

	if(!script_parse_args(L, "u", LUA_UDATA("fs2.ship", lsp)))
		return 0;

	if(lsp->shipp->ai_index != -1)
	{
		ai_info *aip= &Ai_info[lsp->shipp->ai_index];
		if(aip->target_objnum && Objects[aip->target_objnum].type == OBJ_SHIP)
		{
			lua_ship *lsp = LUA_NEW_OBJ(L, "fs2.shp", lua_ship);
			lsp->shipp = &Ships[Objects[aip->target_objnum].instance];
			return 1;
		}
	}

	//We're done!
	return 0;
}

static const script_lua_obj_func_list lua_msn_ship_funcs[] = {
	{"setSpeed", lua_msn_ship_setSpeed, LUA_OPER_METHOD, "Sets ship speed: (x speed, y speed, z speed)"},
	{"setName", lua_msn_ship_setName, LUA_OPER_METHOD, "Changes ship name: (new name)"},
	{"getTarget", lua_msn_ship_getTarget, LUA_OPER_METHOD, "Returns handle to ship's targetted ship"},
	{SCRIPT_END_LIST},
};

//LIBRARY define
static const script_lua_obj_list lua_msn_lib_obj[] = {
	//{"fs2.lvec", lua_fs2_lvec_funcs},
	{"fs2.ship", lua_msn_ship_funcs, "Ship handle"},
	{SCRIPT_END_LIST},
};
static const script_lua_func_list lua_msn_lib[] = {
	//{"lvec", lua_fs2_lvec},
	{"newShip", lua_msn_newShip, "Creates a new ship and returns a handle to it: (ship name, class name, x pos, y pos, z pos; x rot, y rot, z rot)"},
	{SCRIPT_END_LIST},
};

//**********LIBRARY: Graphics (grl)
static int lua_grpc_setColor(lua_State *L)
{
	if(!Gr_inited)
		return 0;

	int r,g,b,a=255;

	if(!script_parse_args(L, "iii|i", &r, &g, &b, &a))
		return 0;

	color ac;
	gr_init_alphacolor(&ac,r,g,b,a);
	gr_set_color_fast(&ac);

	return 0;
}

static int lua_grpc_setFont(lua_State *L)
{
	if(!Gr_inited)
		return 0;

	int fn;

	if(!script_parse_args(L, "i", &fn))
		return 0;

	gr_set_font(fn);

	return 0;
}

static int lua_grpc_drawPixel(lua_State *L)
{
	if(!Gr_inited)
		return 0;

	int x,y;
	bool r=true;

	if(!script_parse_args(L, "ii|b", &x, &y, &r))
		return 0;

	gr_pixel(x,y,r);

	return 0;
}

static int lua_grpc_drawLine(lua_State *L)
{
	if(!Gr_inited)
		return 0;

	int x1,y1,x2,y2;
	bool r=true;

	if(!script_parse_args(L, "iiii|b", &x1, &y1, &x2, &y2, &r))
		return 0;

	gr_line(x1,y1,x2,y2,r);

	return 0;
}

static int lua_grpc_drawGradientLine(lua_State *L)
{
	if(!Gr_inited)
		return 0;

	int x1,y1,x2,y2;
	bool r=true;

	if(!script_parse_args(L, "iiii|b", &x1, &y1, &x2, &y2, &r))
		return 0;

	gr_gradient(x1,y1,x2,y2,r);

	return 0;
}

static int lua_grpc_drawCircle(lua_State *L)
{
	if(!Gr_inited)
		return 0;

	int x,y,ra;
	bool r=true;

	if(!script_parse_args(L, "iii|b", &x,&y,&ra,&r))
		return 0;

	gr_circle(x,y, ra, r);

	return 0;
}

static int lua_grpc_drawCurve(lua_State *L)
{
	if(!Gr_inited)
		return 0;

	int x,y,ra,d;

	if(!script_parse_args(L, "iiii|b", &x,&y,&ra,&d))
		return 0;

	gr_curve(x,y,ra,d);

	return 0;
}

static int lua_grpc_drawText(lua_State *L)
{
	if(!Gr_inited)
		return 0;

	int x,y;
	char *s;
	bool r=true;

	if(!script_parse_args(L, "iis|b", &x, &y, &s, &r))
		return 0;

	gr_string(x,y,s,r);

	return 0;
}

static int lua_grpc_flashScreen(lua_State *L)
{
	if(!Gr_inited)
		return 0;

	int r,g,b;

	if(!script_parse_args(L, "iii", &r, &g, &b))
		return 0;

	gr_flash(r,g,b);

	return 0;
}

//LIBRARY define
static const script_lua_func_list lua_grpc_lib[] = {
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

//**********LIBRARY Array
//Add an item in here to add an item to scripting.
//Elements: {library name, library function list, library object list}
const script_lua_lib_list Lua_libraries[] = {
	{"grpc", lua_grpc_lib, NULL, "Graphics library"},
	{"misn", lua_msn_lib, lua_msn_lib_obj, "Mission library"},
	{NULL, lua_base_lib, lua_base_lib_obj, "Base FS2 library"},
	{SCRIPT_END_LIST},
};

#endif //USE_LUA
//*************************Housekeeping*************************

//Inits LUA
//Note that "libraries" must end with a {NULL, NULL}
//element
int script_state::CreateLuaState(const script_lua_lib_list *libraries)
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
	const script_lua_func_list* func = NULL;
	const script_lua_obj_list* obj = NULL;
	const script_lua_obj_func_list *ofunc = NULL;
	const script_lua_lib_list* lib = libraries;

	for(; lib->library_funcs != NULL; lib++)
	{
		//If a library name is given, register functions as library items
		//If not, register functions as globals
		if(lib->library_name != NULL && strlen(lib->library_name))
		{
			//Register library functions
			if(lib->library_funcs != NULL)
			{
				//luaL_register(L, lib->library_name, lib->library_funcs);
				//luaL_openlib(L, lib->library_name, lib->library_funcs, 0);
				
				//NOTE FROM WMC:
				//The following is based on luaL_openlib from lauxlib.c
				//The default library can't be used because my custom script
				//function array features a field for function description

				//Check for the library's existence
				lua_pushstring(L, lib->library_name);
				lua_gettable(L, LUA_GLOBALSINDEX);

				//If it doesn't exist...
				if (lua_isnil(L, -1))
				{
					lua_pop(L, 1);									//Pop the nil resultfrom the stack
					lua_newtable(L);								//Create a new table
					lua_pushstring(L, lib->library_name);		//Add a string to the stack
					lua_pushvalue(L, -2);							//Push the table
					lua_settable(L, LUA_GLOBALSINDEX);				//Register the table with the new name
				}

				for(func = lib->library_funcs; func->func != NULL; func++)
				{
					//Add each function
					lua_pushstring(L, func->name);			//Push the function's name onto the stack
					lua_pushcclosure(L, func->func, 0);		//Push the function pointer onto the stack
					lua_settable(L, -3);					//Add it into the current lib table
				}
			}
		}
		else
		{
			//Iterate through the function list
			for(func = lib->library_funcs; func->func != NULL; func++)
			{
				//Sanity checking
				Assert(func->name != NULL && strlen(func->name));
				Assert(func->func != NULL);

				//Register the function with the name given as a global
				lua_register(L, func->name, func->func);
			}
		}

		//Handle objects and their methods in a library
		if(lib->library_objects != NULL)
		{
			for(obj = (script_lua_obj_list*)lib->library_objects; obj->object_funcs != NULL; obj++)
			{
				if(!luaL_newmetatable(L, obj->object_name))
				{
					Warning(LOCATION, "Couldn't create metatable for object '%s'", obj->object_name);
					continue;
				}
				//Get the absolute position of the object metatable for later use
				int table_loc = lua_gettop(L);

				ofunc = (script_lua_obj_func_list*)obj->object_funcs;

				//***Check for methods and indexes at the same time, I think these are invalid
				bool methods = false;

				for (; ofunc->name; ofunc++)
				{
					if(ofunc->type == LUA_OPER_METHOD)
					{
						methods = true;
						break;
					}
				}

				if(methods)
				{
					for(ofunc = (script_lua_obj_func_list*)obj->object_funcs; ofunc->name; ofunc++)
					{
						if(ofunc->type == LUA_OPER_INDEX && methods)
						{
								Error(LOCATION, "Attempt to use both methods and indexing in \"%s\"; contact a coder", obj->object_name);
						}
					}
				}

				//***Add the functions into the metatables
				bool table_set;
				uint oper_max = sizeof(Lua_opers)/sizeof(char*);

				for(uint i = 0; i < oper_max; i++)
				{
					table_set = false;
					for (ofunc = (script_lua_obj_func_list*)obj->object_funcs; ofunc->name; ofunc++)
					{
						if(ofunc->type == i && i == LUA_OPER_METHOD)
						{
							if(!table_set)
							{
								lua_pushstring(L, Lua_opers[i]);
								lua_pushvalue(L, table_loc);  // pushes the metatable
								lua_settable(L, table_loc);  // metatable.__index = metatable
								table_set = true;
							}
							lua_pushstring(L, ofunc->name);
							lua_pushcclosure(L, ofunc->func, 0);
							lua_settable(L, -3);
						}
						else if(ofunc->type == i)
						{
							//TODO: Get this working
							/*lua_pushstring(L, Lua_opers[i]);
							lua_pushcfunction(L, ofunc->func);
							lua_settable(L, -2);*/
						}
					}
				}
			}
		}
	}
	SetLuaSession(L, libraries);

	return 1;
#else
	return 0;
#endif
}

#ifdef USE_LUA
void output_lib_meta(FILE *fp, const script_lua_lib_list *lib)
{
	const script_lua_func_list *func;
	const script_lua_obj_func_list *ofunc;
	const script_lua_obj_list *obj;

	if(lib->library_funcs != NULL)
	{
		fputs("<dd><dl>", fp);
		for(func = lib->library_funcs; func->name != NULL; func++)
		{
			fprintf(fp, "<dt><b>%s()</b></dt><dd>%s</dd>", func->name, func->description);
		}
		fputs("</dl></dd>", fp);
	}

	if(lib->library_objects != NULL)
	{
		fputs("<dd><dl>", fp);
		for(obj = lib->library_objects; obj->object_name != NULL; obj++)
		{
			fprintf(fp, "<dt id=\"%s.%s\"><h3>%s</h3></dt>", lib->library_name, obj->object_name, obj->object_name);
			fputs("<dd><dl>", fp);
			for(ofunc = obj->object_funcs; ofunc->name != NULL; ofunc++)
			{
				fprintf(fp, "<dt><b>%s()</b></dt><dd>%s</dd>", ofunc->name, ofunc->description);
			}
			fputs("</dl></dd>", fp);
		}
		fputs("</dl></dd>", fp);
	}
}
#endif

void script_state::OutputLuaMeta(FILE *fp)
{
#ifdef USE_LUA
	const script_lua_lib_list *lib;
	const script_lua_obj_list *obj;
	bool unnamed_display = false;

	//***Output quick library-object list
	//Output objects of non-named libraries
	fputs("<dl>", fp);
	for(lib = LuaLibs; lib->library_funcs != NULL; lib++)
	{
		if(lib->library_name != NULL && strlen(lib->library_name))
			continue;

		if(!unnamed_display) {
			fputs("<dt><a href=\"#LuaBase\">Base</a></dt>", fp);
			unnamed_display = true;
		}

		if(lib->library_objects != NULL)
		{
			for(obj = lib->library_objects; obj->object_name != NULL; obj++)
			{
				fprintf(fp, "<dd><a href=\"#%s.%s\">%s</a> - %s</dd>", lib->library_name, obj->object_name, obj->object_name, obj->description);
			}
		}
	}
	//For named libaries
	for(lib = LuaLibs; lib->library_funcs != NULL; lib++)
	{
		if(lib->library_name == NULL || !strlen(lib->library_name))
			continue;

		fprintf(fp, "<dt><a href=\"#%s\">%s</a> - %s</dt>", lib->library_name, lib->library_name, lib->description);
		if(lib->library_objects != NULL)
		{
			for(obj = lib->library_objects; obj->object_name != NULL; obj++)
			{
				fprintf(fp, "<dd><a href=\"#%s.%s\">%s</a> - %s</dd>", lib->library_name, obj->object_name, obj->object_name, obj->description);
			}
		}
	}
	fputs("</dl><br/><br/>", fp);

	//***Output libs
	fputs("<dl>", fp);
	//Unnamed libs
	unnamed_display = false;
	for(lib = LuaLibs; lib->library_funcs != NULL; lib++)
	{
		if(lib->library_name != NULL && strlen(lib->library_name))
			continue;

		if(!unnamed_display)
		{
			fputs("<dt id=\"LuaBase\"><h2>Base</h2></dt>", fp);
			unnamed_display = true;
		}

		output_lib_meta(fp, lib);
	}
	//Named libs
	for(lib = LuaLibs; lib->library_funcs != NULL; lib++)
	{
		if(lib->library_name == NULL || !strlen(lib->library_name))
			continue;

		fprintf(fp, "<dt id=\"%s\"><h2>%s - %s</h2></dt>", lib->library_name, lib->library_name, lib->description);

		output_lib_meta(fp, lib);
	}
	fputs("</dl>", fp);
#endif
}