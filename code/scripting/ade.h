//
//

#ifndef FS2_OPEN_ADE_H
#define FS2_OPEN_ADE_H

#include "globalincs/pstypes.h"
#include "platformChecks.h"

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
// Value fo ade_odata::size for when buf contains a pointer
const size_t ODATA_PTR_SIZE = (size_t) -1;

const int ADE_FUNCNAME_UPVALUE_INDEX = 1;
const int ADE_SETTING_UPVALUE_INDEX = 2;
const int ADE_DESTRUCTOR_OBJ_UPVALUE_INDEX = 3; // Upvalue which stores the reference to the ade_obj of a destructor
#define ADE_SETTING_VAR lua_toboolean(L,lua_upvalueindex(ADE_SETTING_UPVALUE_INDEX))

template <typename T>
struct ade_odata_getter {
	size_t idx;
	T* value_ptr;

	ade_odata_getter(size_t idx_in, T* ptr_in) : idx(idx_in), value_ptr(ptr_in) {}
};

template <typename T>
struct ade_odata_ptr_getter {
	size_t idx;
	T** value_ptr;

	ade_odata_ptr_getter(size_t idx_in, T** ptr_in) : idx(idx_in), value_ptr(ptr_in) {}
};

template <typename T>
struct ade_odata_setter {
	size_t idx;
	T value;

	ade_odata_setter(size_t idx_in, T&& value_in) : idx(idx_in), value(std::move(value_in)) {}
	ade_odata_setter(size_t idx_in, const T& value_in) : idx(idx_in), value(value_in) {}
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
	const char* Name = nullptr;
	const char* ShortName = nullptr;

	//Important stuff
	size_t Idx = UINT_MAX;
	size_t ParentIdx = UINT_MAX;
	size_t DerivatorIdx = UINT_MAX;
	//ade_id AdeID;
	//ade_id DerivatorID;			//Who do we derive from

	//Type-specific
	bool Instanced = false;                //Is this a single instance?
	char Type = '\0';

	//Functions/virtfuncs
	lua_CFunction Function = nullptr;

	// For Objects, the destructor of the object
	void* Destructor_upvalue = nullptr;
	lua_CFunction Destructor = nullptr;

	size_t Size = 0;

	//Metadata
	const char* Arguments = nullptr;
	const char* Description = nullptr;
	const char* ReturnType = nullptr;
	const char* ReturnDescription = nullptr;

	//Subentries, of course
	//WMC - I have HAD it with these motherfriendly vectors
	//on this motherfriendly class.
	size_t Num_subentries = 0;
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
 * @ingroup ade_api
 */
int ade_set_object_with_breed(lua_State *L, int obj_idx);

/**
 * @brief Loads and executes a default lua script
 *
 * This uses the specified file name and either retrieves it from the default files or uses it as a file name if the mod
 * option is enabled.
 *
 * @param L The lua state
 * @param name The name of the script file
 *
 * @ingroup ade_api
 */
void load_default_script(lua_State* L, const char* name);

namespace internal {

ade_table_entry& getTableEntry(size_t idx);
}
}

#endif //FS2_OPEN_ADE_H
