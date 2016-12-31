//
//

#ifndef FS2_OPEN_ADE_H
#define FS2_OPEN_ADE_H

#include "globalincs/pstypes.h"

extern "C" {
#include <lauxlib.h>
#include <lualib.h>
}

/**
 * @defgroup ade_api ADE API functions
 *
 * @brief Functions and macros used in the ADE scripting API
 *
 * These functions enable the code to communicate with external scripts and expose an API for them to use
 */

namespace scripting {

//*************************Lua funcs*************************
//Used to parse arguments on the stack to C values

/**
 *
 * @param L
 * @param fmt
 * @return
 *
 * @ingroup ade_api
 */
int ade_get_args(lua_State* L, const char* fmt, ...);

/**
 *
 * @param L
 * @param fmt
 * @return
 *
 * @ingroup ade_api
 */
int ade_set_args(lua_State* L, const char* fmt, ...);

/**
 *
 * @param L
 * @param stackdump
 *
 * @ingroup ade_api
 */
void ade_stackdump(lua_State* L, char* stackdump);

/**
 *
 * @param L
 * @return
 *
 * @ingroup ade_api
 */
int ade_friendly_error(lua_State* L);

//*************************Lua types*************************

//WMC - Define to say that this is to store just a pointer.
typedef uint32_t ODATA_SIG_TYPE; //WMC - Please don't touch.
const size_t ODATA_PTR_SIZE = (size_t) -1;
const ODATA_SIG_TYPE ODATA_SIG_DEFAULT = 0;


const int ADE_FUNCNAME_UPVALUE_INDEX = 1;
const int ADE_SETTING_UPVALUE_INDEX = 2;
#define ADE_SETTING_VAR lua_toboolean(L,lua_upvalueindex(ADE_SETTING_UPVALUE_INDEX))

/** Used for internal object->lua_set and lua_get->object communication.
 * Must remain a struct and only contain POD datatypes because this is passed via
 * variable args.
 *
 * @ingroup ade_api
*/
struct ade_odata {
	//ade_id aid;
	size_t idx;
	ODATA_SIG_TYPE* sig;
	void* buf;
	size_t size;
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

/**
 * @ingroup ade_api
 */
class ade_table_entry {
 public:
	const char* Name;
	const char* ShortName;

	//Important stuff
	size_t Idx;
	size_t ParentIdx;
	size_t DerivatorIdx;
	//ade_id AdeID;
	//ade_id DerivatorID;			//Who do we derive from

	//Type-specific
	bool Instanced;                //Is this a single instance?
	char Type;
	union {
		//Variables
		bool varBool;
		double varDouble;
		float varFloat;
		int varInt;
		char* varString;

		//Functions/virtfuncs
		lua_CFunction Function;

		//Objects
		ade_odata Object;
	} Value;
	size_t Size;

	//Metadata
	const char* Arguments;
	const char* Description;
	const char* ReturnType;
	const char* ReturnDescription;

	//Subentries, of course
	//WMC - I have HAD it with these motherfriendly vectors
	//on this motherfriendly class.
	size_t Num_subentries;
	size_t Subentries[256];

 private:
	//*****Internal functions
	//int IndexHandler(lua_State *L);

 public:
	//*****Constructors
	ade_table_entry();

	//*****Operators
	//ade_table_entry &operator = (const ade_table_entry &ate);

	//*****Functions
	size_t AddSubentry(ade_table_entry& n_ate);
	int SetTable(lua_State* L, int p_amt_ldx, int p_mtb_ldx);
	void OutputMeta(FILE* fp);

	//*****Get
	const char* GetName() { if (Name != NULL) { return Name; } else { return ShortName; }}
};

/**
 * @ingroup ade_api
 */
class ade_manager {
	SCP_vector<ade_table_entry> _table_entries;

	ade_manager() {}
 public:
	static ade_manager* getInstance();

	// Disallow copying
	ade_manager(const ade_manager&) = delete;
	ade_manager& operator=(const ade_manager&) = delete;

	// Disallow moving
	ade_manager(ade_manager&&) = delete;
	ade_manager& operator=(ade_manager&&) = delete;

	size_t addTableEntry(const ade_table_entry& entry);

	ade_table_entry& getEntry(size_t idx);
	const ade_table_entry& getEntry(size_t idx) const;

	size_t getNumEntries() const { return _table_entries.size(); }
};

/**
 * @ingroup ade_api
 */
class ade_lib_handle {
 protected:
	size_t LibIdx;
 public:
	ade_lib_handle() {}

	size_t GetIdx() const {
		return LibIdx;
	}
};

/**
 * @ingroup ade_api
 */
template<class StoreType>
class ade_obj: public ade_lib_handle {
 public:
	ade_obj(const char* in_name, const char* in_desc, const ade_lib_handle* in_deriv = NULL) {
		ade_table_entry ate;

		//WMC - object metadata are uninstanced library types
		ate.Name = in_name;
		if (in_deriv != NULL) {
			ate.DerivatorIdx = in_deriv->GetIdx();
		}
		ate.Type = 'o';
		ate.Description = in_desc;
		ate.Value.Object.idx = ade_manager::getInstance()->getNumEntries();
		ate.Value.Object.sig = NULL;
		ate.Value.Object.size = sizeof(StoreType);

		LibIdx = ade_manager::getInstance()->addTableEntry(ate);
	}

	//WMC - Use this to store object data for return, or for setting as a global
	ade_odata Set(const StoreType& obj, ODATA_SIG_TYPE n_sig = ODATA_SIG_DEFAULT) const {
		ade_odata od;
		od.idx = LibIdx;
		od.sig = &n_sig;
		od.buf = (void*) &obj;
		od.size = sizeof(StoreType);
		return od;
	}

	//WMC - Use this to copy object data, for modification or whatever
	ade_odata Get(StoreType* ptr, uint* n_sig = NULL) const {
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
	ade_odata GetPtr(StoreType** ptr) const {
		ade_odata od;
		od.idx = LibIdx;
		od.sig = NULL;
		od.buf = (void**) ptr;
		od.size = ODATA_PTR_SIZE;
		return od;
	}
};

/**
 * Library class
 * This is what you define a variable of to make new libraries
 *
 * @ingroup ade_api
 */
class ade_lib : public ade_lib_handle {
 public:
	ade_lib(const char *in_name, const ade_lib_handle *parent=NULL, const char *in_shortname=NULL, const char *in_desc=NULL);

	const char *GetName() const;
};

/**
 * @ingroup ade_api
 */
class ade_func: public ade_lib_handle {
 public:
	ade_func(const char* name,
			 lua_CFunction func,
			 const ade_lib_handle& parent,
			 const char* args = NULL,
			 const char* desc = NULL,
			 const char* ret_type = NULL,
			 const char* ret_desc = NULL);
};

/**
 * @ingroup ade_api
 */
class ade_virtvar: public ade_lib_handle {
 public:
	ade_virtvar(const char* name,
				lua_CFunction func,
				const ade_lib_handle& parent,
				const char* args = NULL,
				const char* desc = NULL,
				const char* ret_type = NULL,
				const char* ret_desc = NULL);
};

/**
 * @ingroup ade_api
 */
class ade_indexer: public ade_lib_handle {
 public:
	ade_indexer(lua_CFunction func, const ade_lib_handle& parent, const char* args = NULL, const char* desc = NULL,
				const char* ret_type = NULL, const char* ret_desc = NULL);
};

/**
 * @ingroup ade_api
 */
void ade_stackdump(lua_State *L, char *stackdump);

/**
 * @ingroup ade_api
 */
int ade_friendly_error(lua_State *L);

/**
 * @ingroup ade_api
 */
const char *ade_get_type_string(lua_State *L, int argnum);

/**
 * @brief Sets an object parameter with the right type
 *
 * This should be used everywhere where an object value is returned to make sure that the scripter has access to
 * all API functions.
 *
 * @param L The lua state
 * @param obj_idx The object number
 * @return The return value of ade_set_args
 *
 * @author WMC
 */
int ade_set_object_with_breed(lua_State *L, int obj_idx);
}

#endif //FS2_OPEN_ADE_H
