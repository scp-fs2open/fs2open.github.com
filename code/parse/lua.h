#ifndef _LUA_H
#define _LUA_H

extern "C" {
	#include "lauxlib.h"
	#include "lualib.h"
}

#include "object/object.h"
#include "globalincs/pstypes.h"

#include <vector>

//*************************Lua funcs*************************
//Used to parse arguments on the stack to C values
int lua_get_args(lua_State *L, char *fmt, ...);
int lua_set_args(lua_State *L, char* fmt, ...);
void lua_stackdump(lua_State *L, char *stackdump);
int lua_friendly_error(lua_State *L);

//*************************Lua hacks*************************
//WMC - Hack to allow for quick&easy return value parsing
extern int Lua_get_args_skip;

//*************************Lua types*************************
//WMC - These should really all be internal, but I needed lua_obj

//WMC - Define to say that this is to store just a pointer.
#define ODATA_PTR_SIZE		-1
//script_lua_odata Used for internal object->lua_set and lua_get->object communication
struct script_lua_odata
{
	int meta;
	void *buf;
	int size;
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
	char *ShortName;
	char *Description;
	int Derivator;

	std::vector<lua_func_hh> Functions;
	std::vector<lua_var_hh> Variables;

	lua_CFunction Indexer;
	char *IndexerDescription;

public:
	lua_lib_h(char *in_name,  char *in_shortname=NULL, char *in_desc=NULL, int in_deriv=-1){Name = in_name; ShortName=in_shortname; Description = in_desc; Derivator = in_deriv; Indexer = NULL;}
};

extern std::vector<lua_lib_h> lua_Libraries;
extern std::vector<lua_lib_h> lua_Objects;

//Library class
//This is what you define a variable of to make new libraryes
class lua_lib {
private:
	int lib_idx;
public:
	lua_lib(char *in_name, char *in_shortname=NULL, char *in_desc=NULL) {
		lua_Libraries.push_back(lua_lib_h(in_name, in_shortname, in_desc));
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
		lua_Objects.push_back(lua_lib_h(in_name, NULL, in_desc, in_deriv == NULL ? -1 : in_deriv->obj_idx));	//WMC - Handle NULL case
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
	lua_indexer_h(lua_CFunction func, lua_lib &lib, char *args=NULL, char *retvals=NULL, char *desc=NULL) {
		lib.SetIndexer(func, desc);

		//Add function for meta
		lua_func_hh f={0};
		f.Name = "__indexer";
		f.Function = NULL;
		f.Arguments = args;
		f.ReturnValues = retvals;
		f.Description = desc;

		lib.AddFunc(&f);
	}

	lua_indexer_h(lua_CFunction func, lua_obj_h &obj, char *args=NULL, char *retvals=NULL, char *desc=NULL) {
		obj.SetIndexer(func, desc);

		//Add function for meta
		lua_func_hh f={0};
		f.Name = "__indexer";
		f.Function = NULL;
		f.Arguments = args;
		f.ReturnValues = retvals;
		f.Description = desc;
		obj.AddFunc(&f);
	}
};


//Object class
//This is what you define a variable of to make new objects
template <class StoreType> class lua_obj : public lua_obj_h
{
public:
	lua_obj(char*in_name, char*in_desc, lua_obj* in_deriv=NULL):lua_obj_h(in_name, in_desc, in_deriv){};

	//WMC - Use this to store object data for return, or for setting as a global
	script_lua_odata Set(const StoreType &obj) {
		script_lua_odata od;
		od.meta = obj_idx;
		od.buf = (void*)&obj;
		od.size = sizeof(StoreType);
		return od;
	}

	//WMC - Use this to copy object data, for modification or whatever
	script_lua_odata Get(StoreType *ptr){
		script_lua_odata od;
		od.meta = obj_idx;
		od.buf = ptr;
		od.size = sizeof(StoreType);
		return od;
	}

	//WMC - Use this to get a pointer to Lua object data.
	//Use >ONLY< when:
	//1 - You are setting the data of an object (ie 'x' component of vector)
	//2 - To speed up read-only calcs (ie computing dot product of vectors)
	script_lua_odata GetPtr(StoreType **ptr){
		script_lua_odata od;
		od.meta = obj_idx;
		od.buf = (void**)ptr;
		od.size = -1;
		return od;
	}
};

//*************************Lua global structs*************************
struct object_h {
	object *objp;
	int sig;

	bool IsValid(){return (this != NULL && objp->signature == sig);}
	object_h(object *in){objp=in; sig=in->signature;}
};

//*************************Lua globals*************************
extern lua_obj<object_h> l_Object;
extern lua_obj<object_h> l_Weapon;
extern lua_obj<object_h> l_Ship;

#endif //_LUA_H
