
#include "scripting/ade_args.h"
#include "scripting/ade.h"

#include "scripting/lua/LuaTable.h"
#include "scripting/lua/LuaFunction.h"
#include "scripting/lua/LuaConvert.h"

namespace {
using namespace scripting;

ade_table_entry& getTableEntry(size_t idx) {
	return ade_manager::getInstance()->getEntry(idx);
}
}

namespace scripting {

//WMC - hack to skip X number of arguments on the stack
//Lets me use ade_get_args for global hook return values
int Ade_get_args_skip = 0;
bool Ade_get_args_lfunction = false;

//ade_get_args(state, arguments, variables)
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
int ade_get_args(lua_State *L, const char *fmt, ...)
{
	//Check that we have all the arguments that we need
	//If we don't, return 0
	int needed_args = (int)strlen(fmt);
	int total_args = lua_gettop(L) - Ade_get_args_skip;

	if(strchr(fmt, '|') != NULL) {
		needed_args = (int)(strchr(fmt, '|') - fmt);
	}

	char funcname[128] = "\0";
#ifndef NDEBUG
	lua_Debug ar;
	memset(&ar, 0, sizeof(ar));
	if(lua_getstack(L, 0, &ar))
	{
		lua_getinfo(L, "nl", &ar);
		strcpy_s(funcname, "");
		if(ar.name != NULL) {
			strcat_s(funcname, ar.name);
		}
		if(ar.currentline > -1) {
			char buf[33];
			sprintf(buf, "%d", ar.currentline);
			strcat_s(funcname, " (Line ");
			strcat_s(funcname, buf);
			strcat_s(funcname, ")");
		}
	}
#endif
	if(!strlen(funcname)) {
		//WMC - This was causing crashes with user-defined functions.
		//WMC - Try and get at function name from upvalue
		if(!Ade_get_args_lfunction && !lua_isnone(L, lua_upvalueindex(ADE_FUNCNAME_UPVALUE_INDEX)))
		{
			if(lua_type(L, lua_upvalueindex(ADE_FUNCNAME_UPVALUE_INDEX)) == LUA_TSTRING)
				strcpy_s(funcname, lua_tostring(L, lua_upvalueindex(ADE_FUNCNAME_UPVALUE_INDEX)));
		}

		//WMC - Totally unknown function
		if(!strlen(funcname)) {
			strcpy_s(funcname, "<UNKNOWN>");
		}
	}
	if(total_args < needed_args) {
		LuaError(L, "Not enough arguments for '%s' - need %d, had %d. If you are using objects or handles, make sure that you are using \":\" to access member functions, rather than \".\"", funcname, needed_args, total_args);
		return 0;
	}

	//Start throught
	va_list vl;
	int nargs;
	int counted_args = 0;

	//Are we parsing optional args yet?
	bool optional_args = false;

	va_start(vl, fmt);
	nargs = 1 + Ade_get_args_skip;
	total_args += Ade_get_args_skip;
	while(*fmt && nargs <= total_args)
	{
		switch(*fmt++)
		{
			case 'b':
				if(lua_isboolean(L, nargs)) {
					*va_arg(vl, bool*) = lua_toboolean(L, nargs) > 0 ? true : false;
				} else {
					LuaError(L, "%s: Argument %d is an invalid type '%s'; boolean expected", funcname, nargs, ade_get_type_string(L, nargs));
					if(!optional_args) {
						va_end(vl);
						return 0;
					}
				}
				break;
			case 'd':
				if(lua_isnumber(L, nargs)) {
					*va_arg(vl, double*) = (double)lua_tonumber(L, nargs);
				} else {
					LuaError(L, "%s: Argument %d is an invalid type '%s'; number expected", funcname, nargs, ade_get_type_string(L, nargs));
					if(!optional_args) {
						va_end(vl);
						return 0;
					}
				}
				break;
			case 'f':
				if(lua_isnumber(L, nargs)) {
					*va_arg(vl, float*) = (float)lua_tonumber(L, nargs);
				} else {
					LuaError(L, "%s: Argument %d is an invalid type '%s'; number expected", funcname, nargs, ade_get_type_string(L, nargs));
					if(!optional_args) {
						va_end(vl);
						return 0;
					}
				}
				break;
			case 'i':
				if(lua_isnumber(L, nargs)) {
					*va_arg(vl, int*) = (int)lua_tonumber(L, nargs);
				} else {
					LuaError(L, "%s: Argument %d is an invalid type '%s'; number expected", funcname, nargs, ade_get_type_string(L, nargs));
					if(!optional_args) {
						va_end(vl);
						return 0;
					}
				}
				break;
			case 's':
				if(lua_isstring(L, nargs)) {
					*va_arg(vl, const char **) = lua_tostring(L, nargs);
				} else {
					LuaError(L, "%s: Argument %d is an invalid type '%s'; string expected", funcname, nargs, ade_get_type_string(L, nargs));
					if(!optional_args) {
						va_end(vl);
						return 0;
					}
				}
				break;
			case 'x':
				if(lua_isnumber(L, nargs)) {
					*va_arg(vl, fix*) = fl2f((float)lua_tonumber(L, nargs));
				} else {
					LuaError(L, "%s: Argument %d is an invalid type '%s'; number expected", funcname, nargs, ade_get_type_string(L, nargs));
					if(!optional_args) {
						va_end(vl);
						return 0;
					}
				}
				break;
			case 'o':
			{
				ade_odata od = va_arg(vl, ade_odata);
				if(lua_isuserdata(L, nargs))
				{
					//WMC - Get metatable
					lua_getmetatable(L, nargs);
					int mtb_ldx = lua_gettop(L);
					Assert(!lua_isnil(L, -1));

					//Get ID
					lua_pushstring(L, "__adeid");
					lua_rawget(L, mtb_ldx);

					if(lua_tonumber(L, -1) != od.idx)
					{
						lua_pushstring(L, "__adederivid");
						lua_rawget(L, mtb_ldx);
						if((uint)lua_tonumber(L, -1) != od.idx)
						{
							LuaError(L, "%s: Argument %d is the wrong type of userdata; '%s' given, but '%s' expected", funcname, nargs, getTableEntry((uint)lua_tonumber(L, -2)).Name, getTableEntry(od.idx).GetName());
							if(!optional_args) {
								va_end(vl);
								return 0;
							}
						}
						lua_pop(L, 1);
					}
					lua_pop(L, 2);
					if(od.size != ODATA_PTR_SIZE)
					{
						memcpy(od.buf, lua_touserdata(L, nargs), od.size);
						if(od.sig != NULL) {
							//WMC - char must be 1
							Assert(sizeof(char) == 1);
							//WMC - Yuck. Copy sig data.
							//Maybe in the future I'll do a packet userdata thing.
							(*od.sig) = *(ODATA_SIG_TYPE*)(*(char **)od.buf + od.size);
						}
					} else {
						(*(void**)od.buf) = lua_touserdata(L, nargs);
					}
				}
				else if(lua_isnil(L, nargs) && optional_args)
				{
					//WMC - Modder has chosen to ignore this argument
				}
				else
				{
					LuaError(L, "%s: Argument %d is an invalid type '%s'; type '%s' expected", funcname, nargs, ade_get_type_string(L, nargs), getTableEntry(od.idx).GetName());
					if(!optional_args) {
						va_end(vl);
						return 0;
					}
				}
			}
				break;
			case 't':
			{
				// Get a table
				try {
					*va_arg(vl, luacpp::LuaTable*) = luacpp::convert::popValue<luacpp::LuaTable>(L, nargs, false);
				} catch (const luacpp::LuaException& e) {
					LuaError(L, "%s: Argument %d is an invalid type '%s'; table expected.", funcname, nargs, ade_get_type_string(L, nargs));
					if(!optional_args) {
						va_end(vl);
						return 0;
					}
				}
				break;
			}
			case 'u':
			{
				// Get a function
				try {
					*va_arg(vl, luacpp::LuaFunction*) = luacpp::convert::popValue<luacpp::LuaFunction>(L, nargs, false);
				} catch (const luacpp::LuaException& e) {
					LuaError(L, "%s: Argument %d is an invalid type '%s'; function expected.", funcname, nargs, ade_get_type_string(L, nargs));
					if(!optional_args) {
						va_end(vl);
						return 0;
					}
				}
				break;
			}
			case '|':
				nargs--;	//cancel out the nargs++ at the end
				counted_args--;
				optional_args = true;
				break;
			case '*':
				//WMC - Ignore one spot
				break;
			default:
				Error(LOCATION, "%s: Bad character passed to ade_get_args; (%c)", funcname, *(fmt-1));
				break;
		}
		nargs++;
		counted_args++;
	}
	va_end(vl);
	return counted_args;
}

//ade_set_args(state, arguments, variables)
//----------------------------------------------
//based on "Programming in Lua"
//
//Takes variables given and pushes them onto the
//Lua stack. Use it to return variables from a
//Lua scripting function.
//
//NOTE: You can also use this to push arguments
//on to the stack in series. See script_state::SetHookVar
int ade_set_args(lua_State *L, const char *fmt, ...)
{
	//Start throught
	va_list vl;
	int nargs;
	int setargs;	//args actually set

	va_start(vl, fmt);
	nargs = 0;
	setargs = 0;
	while(*fmt != '\0')
	{
		switch(*fmt++)
		{
			case '*':
				lua_pushnil(L);
				break;
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
			{
				//WMC - Isn't working with HookVar for some strange reason
				char *s = va_arg(vl, char*);
				lua_pushstring(L, s);
				break;
			}
			case 'x':
				lua_pushnumber(L, f2fl(va_arg(vl, fix)));
				break;
			case 'o':
			{
				//WMC - char must be 1 byte, foo.
				Assert(sizeof(char)==1);
				//WMC - step by step
				//Copy over objectdata
				ade_odata od = (ade_odata) va_arg(vl, ade_odata);

				//Create new LUA object and get handle
				char *newod = (char*)lua_newuserdata(L, od.size + sizeof(ODATA_SIG_TYPE));
				//Create or get object metatable
				luaL_getmetatable(L, getTableEntry(od.idx).Name);
				//Set the metatable for the object
				lua_setmetatable(L, -2);

				//Copy the actual object data to the Lua object
				memcpy(newod, od.buf, od.size);

				//Also copy in the unique sig
				if(od.sig != NULL)
					memcpy(newod + od.size, od.sig, sizeof(ODATA_SIG_TYPE));
				else
				{
					ODATA_SIG_TYPE tempsig = ODATA_SIG_DEFAULT;
					memcpy(newod + od.size, &tempsig, sizeof(ODATA_SIG_TYPE));
				}
				break;
			}
			case 't':
			{
				// Set a table
				luacpp::LuaTable* table = va_arg(vl, luacpp::LuaTable*); // This must be a pointer since C++ classes can't be passed by vararg
				table->pushValue();
				break;
			}
			case 'u':
			{
				// Set a function
				luacpp::LuaFunction* func = va_arg(vl, luacpp::LuaFunction*); // This must be a pointer since C++ classes can't be passed by vararg
				func->pushValue();
				break;
			}
				//WMC -  Don't forget to update lua_set_arg
			default:
				Error(LOCATION, "Bad character passed to ade_set_args; (%c)", *(fmt-1));
				setargs--;
		}
		nargs++;
		setargs++;
	}
	va_end(vl);
	return setargs;
}

}
