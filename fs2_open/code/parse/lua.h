#ifndef _LUA_H
#define _LUA_H
#ifdef USE_LUA

extern "C" {
	#include "lauxlib.h"
	#include "lualib.h"
}

#include <vector>

//*************************Lua funcs*************************
//Used to parse arguments on the stack to C values
int lua_get_args(lua_State *L, char *fmt, ...);
//void lua_set_arg(lua_State *L, char fmt, void *dta);
int lua_set_args(lua_State *L, char* fmt, ...);
void lua_stackdump(lua_State *L, char *stackdump);

//*************************Lua hacks*************************
//WMC - Hack to allow for quick&easy return value parsing
extern int Lua_get_args_skip;

//*************************Lua types*************************
//WMC - These should really all be internal, but I needed lua_obj
//WMC - Types
//Used for internal object->lua_return and lua_parse->object communication
struct script_lua_odata
{
	int meta;
	void *buf;
	int size;

	//script_lua_odata(char *in_meta, void *in_buf, int in_size){meta=in_meta;buf=in_buf;size=in_size;}
};

//Used like _odata, except for object pointers
struct script_lua_opdata
{
	int meta;
	void **buf;

	//script_lua_opdata(char* in_meta, void** in_buf){meta=in_meta; buf=in_buf;}
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

struct lua_var_hh
{
	char *Name;
	bool IsArray;
	lua_CFunction Function;
	char *Type;
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
	int Derivator;

	std::vector<lua_func_hh> Functions;
	std::vector<lua_var_hh> Variables;

	lua_CFunction Indexer;
	char *IndexerDescription;

public:
	lua_lib_h(char *in_name, char *in_desc, int in_deriv=-1){Name = in_name; Description = in_desc; Derivator = in_deriv; Indexer = NULL;}
};

extern std::vector<lua_lib_h> lua_Libraries;
extern std::vector<lua_lib_h> lua_Objects;

//Library class
//This is what you define a variable of to make new libraryes
class lua_lib {
private:
	int lib_idx;
public:
	lua_lib(char *in_name, char *in_desc) {
		lua_Libraries.push_back(lua_lib_h(in_name, in_desc));
		lib_idx = lua_Libraries.size()-1;
	}

	void AddFunc(lua_func_hh *f){lua_Libraries[lib_idx].Functions.push_back(*f);}
	void AddVar(lua_var_hh *v){lua_Libraries[lib_idx].Variables.push_back(*v);}
	void SetIndexer(lua_CFunction func, char *desc){lua_Libraries[lib_idx].Indexer = func; lua_Libraries[lib_idx].IndexerDescription = desc;}
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
	void AddVar(lua_var_hh *v){lua_Objects[obj_idx].Variables.push_back(*v);}
	void SetIndexer(lua_CFunction func, char *desc){lua_Objects[obj_idx].Indexer = func; lua_Objects[obj_idx].IndexerDescription = desc;}
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

class lua_var_h {
public:
	lua_var_h(char *name, lua_CFunction func, lua_lib &lib, bool isarray, char *type=NULL, char *desc=NULL) {
		lua_var_hh v;

		v.Name = name;
		v.IsArray = isarray;
		v.Function = func;
		v.Type = type;
		v.Description = desc;

		lib.AddVar(&v);
	}

	lua_var_h(char *name, lua_CFunction func, lua_obj_h &obj, bool isarray, char *type=NULL, char *desc=NULL) {
		lua_var_hh v;

		v.Name = name;
		v.IsArray = isarray;
		v.Function = func;
		v.Type = type;
		v.Description = desc;

		obj.AddVar(&v);
	}
};

class lua_indexer_h {
public:
	lua_indexer_h(lua_CFunction func, lua_lib &lib, char *desc=NULL) {
		lib.SetIndexer(func, desc);
	}

	lua_indexer_h(lua_CFunction func, lua_obj_h &obj, char *desc=NULL) {
		obj.SetIndexer(func, desc);
	}
};


//Object class
//This is what you define a variable of to make new objects
template <class StoreType> class lua_obj : public lua_obj_h
{
public:
	lua_obj(char*in_name, char*in_desc, lua_obj* in_deriv=NULL):lua_obj_h(in_name, in_desc, in_deriv){};

	//StoreType *Create(lua_State *L){StoreType *ptr = (StoreType*)LUA_NEW_OBJ(L, meta, StoreType); return ptr;}
	script_lua_odata SetToLua(StoreType *obj) {
		script_lua_odata od;
		od.meta = obj_idx;
		od.buf = obj;
		od.size = sizeof(StoreType);
		return od;
	}

	script_lua_odata GetFromLua(StoreType *ptr){
		script_lua_odata od;
		od.meta = obj_idx;
		od.buf = ptr;
		od.size = sizeof(StoreType);
		return od;
	}

	script_lua_opdata GetPtrFromLua(StoreType **ptr){
		script_lua_opdata pd;
		pd.meta = obj_idx;
		pd.buf = ptr;
		return pd;
	}
};

//*************************Lua globals*************************
extern lua_obj<int> l_Ship;
extern lua_obj<int> l_Object;

#endif //USE_LUA
#endif //_LUA_H
