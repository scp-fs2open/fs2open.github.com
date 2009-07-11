#ifndef _LUA_H
#define _LUA_H

extern "C" {
	#include "../../lua/lauxlib.h"
	#include "../../lua/lualib.h"
}

#include "object/object.h"
#include "globalincs/pstypes.h"

//*************************Lua funcs*************************
//Used to parse arguments on the stack to C values
int ade_get_args(lua_State *L, char *fmt, ...);
int ade_set_args(lua_State *L, char* fmt, ...);
void ade_stackdump(lua_State *L, char *stackdump);
int ade_friendly_error(lua_State *L);

//*************************Lua hacks*************************
//WMC - Hack to allow for quick&easy return value parsing
extern int Ade_get_args_skip;
//WMC - Tell ade_get_args it is parsing lua functions,
//which have no upvalues
extern bool Ade_get_args_lfunction;

//*************************Lua types*************************
//WMC - These should really all be internal, but I needed ade_obj
/*
class ade_id
{
private:
	std::vector<uint> Path;
public:
	ade_id(){Path.clear();}
	ade_id(int n){Path.clear();Path.push_back(n);}
	ade_id(ade_id &id, int n){
		Path.resize(id.Path.size());
		memcpy(&Path[0], &id.Path[0], sizeof(uint) * Path.size());
		Path.push_back(n);
	}

	//Operators
	ade_id &operator = (const ade_id &n_aid);
	bool operator == (ade_id &n_aid);

	//Get
	class ade_table_entry *GetATE();
	bool IsUsed(){return (Path.size() > 0);}
	size_t GetSizeInBytes(){return Path.size() * sizeof(uint);}

	//Set
	void Copy(void *dest){memcpy(dest, &Path[0], Path.size() * sizeof(uint));}
};
*/

//WMC - Define to say that this is to store just a pointer.
#define ODATA_PTR_SIZE		-1
#define ODATA_SIG_TYPE		uint						//WMC - Please don't touch.
#define ODATA_SIG_DEFAULT	0
//ade_odata Used for internal object->lua_set and lua_get->object communication
class ade_odata
{
public:
	//ade_id aid;
	uint idx;
	ODATA_SIG_TYPE *sig;
	void *buf;
	int size;
	//ade_odata(){idx=UINT_MAX;sig=NULL;buf=NULL;size=0;}
/*
	ade_odata &operator =(const ade_odata &slo) {
		aid = slo.aid;
		buf = slo.buf;
		size = slo.size;

		return (*this);
	}*/
};

//WMC - 'Type' is the same as ade_set_args,
//plus some extra
//b - boolean
//d - double
//f - float
//i - integer
//s - string
//x - fix
//o - object
//EXTRA:
//l - library	//WMC - no longer exists
//u - function
//v - virtual variable
//
//u - oh wait...

#define ADE_INDEX(ate) (ate - &Ade_table_entries[0])

extern SCP_vector<class ade_table_entry> Ade_table_entries;

class ade_table_entry
{
public:
	char *Name;
	char *ShortName;

	//Important stuff
	uint ParentIdx;
	uint DerivatorIdx;
	//ade_id AdeID;
	//ade_id DerivatorID;			//Who do we derive from

	//Type-specific
	bool Instanced;				//Is this a single instance?
	char Type;
	union {
		//Variables
		bool varBool;
		double varDouble;
		float varFloat;
		int varInt;
		char *varString;

		//Functions/virtfuncs
		lua_CFunction Function;

		//Objects
		ade_odata Object;
	} Value;
	size_t Size;

	//Metadata
	char *Arguments;
	char *Description;
	char *ReturnType;
	char *ReturnDescription;

	//Subentries, of course.
	//std::vector<ade_table_entry> Subentries;
	//std::vector<uint> Subentries;
	//WMC - I have HAD it with these motherfriendly vectors
	//on this motherfriendly class.
	uint Num_subentries;
	uint Subentries[256];
	
private:
	//*****Internal functions
	//int IndexHandler(lua_State *L);

public:
	//*****Constructors
	ade_table_entry(){
		memset(this, 0, sizeof(ade_table_entry));
		ParentIdx = UINT_MAX;
		DerivatorIdx = UINT_MAX;
	}

	//*****Operators
	//ade_table_entry &operator = (const ade_table_entry &ate);

	//*****Functions
	uint AddSubentry(ade_table_entry &n_ate)
	{
		ade_table_entry ate = n_ate;
		ate.ParentIdx = ADE_INDEX(this);
		Ade_table_entries.push_back(ate);
		uint new_idx = Ade_table_entries.size()-1;

		//WMC - Oi. Moving the Ade_table_entries vector
		//invalidates the "this" pointer. Workaround time.
		ade_table_entry *new_this = &Ade_table_entries[ate.ParentIdx];
		uint idx = new_this->Num_subentries++;
		new_this->Subentries[idx] = new_idx;

		return new_idx;
	}
	int SetTable(lua_State *L, int p_amt_ldx, int p_mtb_ldx);
	void OutputMeta(FILE *fp);

	//*****Get
	char *GetName(){if(Name!=NULL)return Name;else return ShortName;}
};

class ade_lib_handle
{
protected:
	//ade_id LibAID;
	uint LibIdx;
public:
	ade_lib_handle(){}
/*
	void AddEntry(ade_table_entry &in_ate) {
		Ade_table_entries[LibIdx].AddSubentry(in_ate);
	}
*/
	uint GetIdx() {
		return LibIdx;
	}
};

//Object class
//This is what you define a variable of to make new objects
template <class StoreType> class ade_obj : public ade_lib_handle
{
public:
	ade_obj(char*in_name, char*in_desc, ade_lib_handle* in_deriv=NULL)
	{
		ade_table_entry ate;

		//WMC - object metadata are uninstanced library types
		ate.Name = in_name;
		if(in_deriv != NULL)
			ate.DerivatorIdx = in_deriv->GetIdx();
		ate.Type = 'o';
		ate.Description = in_desc;
		ate.Value.Object.idx = Ade_table_entries.size();
		ate.Value.Object.sig = NULL;
		ate.Value.Object.size = sizeof(StoreType);

		Ade_table_entries.push_back(ate);
		LibIdx = Ade_table_entries.size() - 1;
	}

	//WMC - Use this to store object data for return, or for setting as a global
	ade_odata Set(const StoreType &obj, ODATA_SIG_TYPE n_sig=ODATA_SIG_DEFAULT) {
		ade_odata od;
		od.idx = LibIdx;
		od.sig = (uint*)&n_sig;
		od.buf = (void*)&obj;
		od.size = sizeof(StoreType);
		return od;
	}

	//WMC - Use this to copy object data, for modification or whatever
	ade_odata Get(StoreType *ptr, uint *n_sig=NULL){
		ade_odata od;
		od.idx = LibIdx;
		od.sig = n_sig;
		od.buf = ptr;
		od.size = sizeof(StoreType);
		return od;
	}

	//WMC - Use this to get a pointer to Lua object data.
	//Use >ONLY< when:
	//1 - You are setting the data of an object (ie 'x' component of vector)
	//2 - To speed up read-only calcs (ie computing dot product of vectors)
	ade_odata GetPtr(StoreType **ptr){
		ade_odata od;
		od.idx = LibIdx;
		od.buf = (void**)ptr;
		od.size = -1;
		return od;
	}
};

//*************************Lua global structs*************************


//*************************Lua globals*************************
extern ade_obj<object_h> l_Object;
extern ade_obj<object_h> l_Weapon;
extern ade_obj<object_h> l_Ship;
extern ade_obj<object_h> l_Debris;
extern ade_obj<object_h> l_Asteroid;

#endif //_LUA_H
