//
//

#ifndef FS2_OPEN_ADE_H
#define FS2_OPEN_ADE_H

#include "globalincs/pstypes.h"
#include "globalincs/version.h"

#include "object/object.h"
#include "scripting/ade_doc.h"

extern "C" {
#include <lauxlib.h>
#include <lualib.h>
}

#include <memory>

/**
 * @defgroup ade_api ADE API functions
 *
 * @brief Functions and macros used in the ADE scripting API
 *
 * These functions enable the code to communicate with external scripts and expose an API for them to use
 */


namespace scripting {

// Forward definition
struct DocumentationElement;

using DocumentationErrorReporter = std::function<void(const SCP_string& errorMessage)>;

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

	ade_odata_setter(size_t idx_in, T value_in) : idx(idx_in), value(std::move(value_in)) {}
};

class ade_table_entry;

using ade_serialize_func = void(*)(lua_State*, const scripting::ade_table_entry&, const luacpp::LuaValue&, ubyte*, int&);
using ade_deserialize_func = void(*)(lua_State*, const scripting::ade_table_entry&, char*, ubyte*, int&);

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
	ade_serialize_func Serializer = nullptr;
	ade_deserialize_func Deserializer = nullptr;

	//Metadata
	ade_overload_list Arguments;
	const char* Description = nullptr;
	const char* ReturnType;
	const char* ReturnDescription = nullptr;
	gameversion::version DeprecationVersion;
	const char* DeprecationMessage = nullptr;

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
	std::unique_ptr<DocumentationElement> ToDocumentationElement(
		const scripting::DocumentationErrorReporter& errorReporter);

	//*****Get
	const char* GetName() const;

	SCP_string GetFullPath() const;
};

/**
 * @ingroup ade_api
 */
class ade_manager {
	SCP_vector<ade_table_entry> _table_entries;

	SCP_vector<SCP_string> _type_names;

	ade_manager();
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

	const SCP_vector<SCP_string>& getTypeNames() const;
};

/**
 * @ingroup ade_api
 */
void ade_stackdump(lua_State *L, char *stackdump);

/**
 * @ingroup ade_api
 */
int ade_friendly_error(lua_State* L);

/**
 * @ingroup ade_api
 */
const char* ade_get_type_string(lua_State* L, int argnum);

/**
 * @ingroup ade_api
 */
bool ade_is_internal_type(const char* typeName);

/**
 * @brief Converts an object index to something that can be used with ade_set_args.
 *
 * This respects the actual type of the object so all appropriate functions are available in Lua.
 *
 * @warning This is only used internally and should not be used by API code. Use ade_set_object_with_breed instead.
 *
 * @param obj_idx The object index
 * @return The ade odata
 */
ade_odata_setter<object_h> ade_object_to_odata(int obj_idx);

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
int ade_set_object_with_breed(lua_State* L, int obj_idx);

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

//Struct for converting one string for another. whee!
struct string_conv {
	const char *src;
	const char *dest;
};

const string_conv* ade_get_operator(const char *funcname);

namespace internal {

ade_table_entry& getTableEntry(size_t idx);
}
} // namespace scripting

#endif // FS2_OPEN_ADE_H
