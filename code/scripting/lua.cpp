
#include "scripting/scripting.h"

#include "scripting_doc.h"

#include "scripting/lua/LuaUtil.h"

extern "C" {
#include "scripting/lua/lua_ext.h"

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <sys/stat.h>
#endif
}

/**
 * IMPORTANT!
 *
 * For various compiler reasons, all scripting API classes and libraries need to be referenced here! The easiest way to
 * do this is to declare the API elements in the header with the DECLARE_* macros and then include them here
 */

#include "scripting/api/objs/asteroid.h"
#include "scripting/api/objs/background_element.h"
#include "scripting/api/objs/beam.h"
#include "scripting/api/objs/camera.h"
#include "scripting/api/objs/cockpit_display.h"
#include "scripting/api/objs/control_info.h"
#include "scripting/api/objs/debris.h"
#include "scripting/api/objs/enums.h"
#include "scripting/api/objs/event.h"
#include "scripting/api/objs/eye.h"
#include "scripting/api/objs/file.h"
#include "scripting/api/objs/font.h"
#include "scripting/api/objs/gameevent.h"
#include "scripting/api/objs/gamestate.h"
#include "scripting/api/objs/hudgauge.h"
#include "scripting/api/objs/mc_info.h"
#include "scripting/api/objs/message.h"
#include "scripting/api/objs/model.h"
#include "scripting/api/objs/object.h"
#include "scripting/api/objs/option.h"
#include "scripting/api/objs/order.h"
#include "scripting/api/objs/particle.h"
#include "scripting/api/objs/model_path.h"
#include "scripting/api/objs/physics_info.h"
#include "scripting/api/objs/player.h"
#include "scripting/api/objs/promise.h"
#include "scripting/api/objs/rpc.h"
#include "scripting/api/objs/sexpvar.h"
#include "scripting/api/objs/shields.h"
#include "scripting/api/objs/ship.h"
#include "scripting/api/objs/ship_bank.h"
#include "scripting/api/objs/shipclass.h"
#include "scripting/api/objs/shiptype.h"
#include "scripting/api/objs/sound.h"
#include "scripting/api/objs/species.h"
#include "scripting/api/objs/streaminganim.h"
#include "scripting/api/objs/subsystem.h"
#include "scripting/api/objs/team.h"
#include "scripting/api/objs/texture.h"
#include "scripting/api/objs/texturemap.h"
#include "scripting/api/objs/time_obj.h"
#include "scripting/api/objs/vecmath.h"
#include "scripting/api/objs/waypoint.h"
#include "scripting/api/objs/weapon.h"
#include "scripting/api/objs/weaponclass.h"
#include "scripting/api/objs/wing.h"

#include "scripting/api/libs/audio.h"
#include "scripting/api/libs/async.h"
#include "scripting/api/libs/base.h"
#include "scripting/api/libs/bitops.h"
#include "scripting/api/libs/cfile.h"
#include "scripting/api/libs/controls.h"
#include "scripting/api/libs/engine.h"
#include "scripting/api/libs/globals.h"
#include "scripting/api/libs/graphics.h"
#include "scripting/api/libs/hookvars.h"
#include "scripting/api/libs/hud.h"
#include "scripting/api/libs/mission.h"
#include "scripting/api/libs/multi.h"
#include "scripting/api/libs/options.h"
#include "scripting/api/libs/parse.h"
#include "scripting/api/libs/tables.h"
#include "scripting/api/libs/testing.h"
#include "scripting/api/libs/time_lib.h"
#include "scripting/api/libs/ui.h"
#include "scripting/api/libs/utf8.h"

// End of definitions includes

using namespace scripting;
using namespace scripting::api;

namespace {
const char* ScriptStateReferenceName = "SCP_ScriptState";
}

// *************************Housekeeping*************************

static void *vm_lua_alloc(void*, void *ptr, size_t, size_t nsize) {
	if (nsize == 0)
	{
		vm_free(ptr);
		return NULL;
	}
	else
	{
		return vm_realloc(ptr, nsize);
	}
}

//kind of fake, prevents true file access (only allows pipes and stuff) and also returns nil on fail instead of error handling string
static int io_open_limited (lua_State *L) {
	const char *filename = luaL_checkstring(L, 1);
	const char *mode = luaL_optstring(L, 2, "r");

	if (filename == nullptr)
		return 0;

	//We allow fifo-pipes, and direct character access. Neither should be too risky, and they allow for features such as communicating with speedrun tools or specialized hardware
	//This explicitly blocks access to files, directories, and block devices
#ifdef WIN32
	auto handle = CreateFileA(filename, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
	bool file_allowed = false;
	if (handle != INVALID_HANDLE_VALUE) {
		auto file_type = GetFileType(handle);
		CloseHandle(handle);
		file_allowed = (file_type == FILE_TYPE_PIPE) || (file_type == FILE_TYPE_CHAR);
	}
#else
	struct stat file_stat_buffer;
	int query_stat = stat( filename, &file_stat_buffer );
	bool file_allowed = (query_stat == 0) && (S_ISFIFO(file_stat_buffer.st_mode) || S_ISCHR(file_stat_buffer.st_mode));
#endif

	//Check that our requested file is nice and not evil
	if (file_allowed) {
		FILE **pf = (FILE **) lua_newuserdata(L, sizeof(FILE *));
		*pf = nullptr;  /* file handle is currently `closed' */
		luaL_getmetatable(L, LUA_FILEHANDLE);
		lua_setmetatable(L, -2);
		*pf = fopen(filename, mode);

		if (*pf != nullptr) {
			return 1;
		}

		lua_pop(L, 1);
	}

	return 0;
}

//Inits LUA
//Note that "libraries" must end with a {NULL, NULL}
//element
int script_state::CreateLuaState()
{
	mprintf(("LUA: Opening LUA state...\n"));
	lua_State *L = lua_newstate(vm_lua_alloc, nullptr);

	if(L == NULL)
	{
		Warning(LOCATION, "Could not initialize Lua");
		return 0;
	}

	//*****INITIALIZE AUXILIARY LIBRARIES
	mprintf(("LUA: Initializing base Lua libraries...\n"));
	luaL_openlibs(L);

	//*****INITIALIZE EXTENSION LIBRARIES
	mprintf(("LUA: Initializing extended Lua libraries...\n"));
	luaL_openlibs_ext(L);

	//*****INITIALIZE OUR SUPPORT LIBRARY
	luacpp::util::initializeLuaSupportLib(L);

	// Store our script state pointer in this Lua state so that it can be retrieved from the Lua API without depending
	// on global state
	lua_pushlightuserdata(L, this);
	lua_setfield(L, LUA_REGISTRYINDEX, ScriptStateReferenceName);

	//*****DISABLE DANGEROUS COMMANDS
	lua_pushstring(L, "os");
	lua_rawget(L, LUA_GLOBALSINDEX);
	int os_ldx = lua_gettop(L);
	if(lua_istable(L, os_ldx))
	{
		lua_pushstring(L, "execute");
		lua_pushnil(L);
		lua_rawset(L, os_ldx);
		lua_pushstring(L, "remove");
		lua_pushnil(L);
		lua_rawset(L, os_ldx);
		lua_pushstring(L, "rename");
		lua_pushnil(L);
		lua_rawset(L, os_ldx);
	}
	lua_pop(L, 1);	//os table

	lua_pushstring(L, "io");
	lua_rawget(L, LUA_GLOBALSINDEX);
	int io_ldx = lua_gettop(L);
	if(lua_istable(L, io_ldx))
	{
		lua_pushstring(L, "popen");
		lua_pushnil(L);
		lua_rawset(L, io_ldx);

		//Instead of just removing open alltogether. Make sure to copy over the original fenv, as to allow lua to have the correct close already associated
		lua_pushstring(L, "open"); // [io, "open"]
		lua_pushstring(L, "open"); // [io, "open", "open"]
		lua_rawget(L, io_ldx); // [io, "open", open()]
		lua_pushcfunction(L, io_open_limited); // [io, "open", open(), open_limited()]
		lua_getfenv(L, -2); // [io, "open", open(), open_limited(), open_fenv]
		lua_setfenv(L, -2); // [io, "open", open(), open_limited()]
		lua_remove(L, -2); // [io, "open", open_limited()]
		lua_rawset(L, io_ldx); // [io]
	}
	lua_pop(L, 1);	//io table

	//*****INITIALIZE ADE
	uint i;
	mprintf(("LUA: Beginning ADE initialization\n"));
	for(i = 0; i < ade_manager::getInstance()->getNumEntries(); i++)
	{
		//WMC - Do only toplevel table entries, doi
		if(ade_manager::getInstance()->getEntry(i).ParentIdx == UINT_MAX)			//WMC - oh hey, we're done with the meaty point in < 10 lines.
			ade_manager::getInstance()->getEntry(i).SetTable(L, LUA_GLOBALSINDEX, LUA_GLOBALSINDEX);	//Oh the miracles of OOP.
	}

	//*****INITIALIZE ENUMERATION CONSTANTS
	mprintf(("ADE: Initializing enumeration constants...\n"));
	enum_h eh;
	for(i = 0; i < Num_enumerations; i++)
	{
		eh.index = Enumerations[i].def;
		eh.value = Enumerations[i].value;
		eh.is_constant = true;

		ade_set_args(L, "o", l_Enum.Set(eh));
		lua_setglobal(L, Enumerations[i].name);
	}

	//*****ASSIGN LUA SESSION
	mprintf(("ADE: Assigning Lua session...\n"));
	SetLuaSession(L);

	//***** LOAD DEFAULT SCRIPTS
	mprintf(("ADE: Loading default scripts...\n"));
	load_default_script(L, "forwarders.lua");
	load_default_script(L, "cfile_require.lua");
	load_default_script(L, "cfile_include.lua");
	load_default_script(L, "dkjson.lua");

	return 1;
}

static bool sort_table_entries(const ade_table_entry* left, const ade_table_entry* right) {
	const char* leftCmp = left->Name != nullptr ? left->Name : left->ShortName;
	const char* rightCmp = right->Name != nullptr ? right->Name : left->ShortName;

	if (leftCmp == nullptr) {
		return false;
	}
	if (rightCmp == nullptr) {
		return false;
	}

	return stricmp(leftCmp, rightCmp) < 0;
}

static bool sort_doc_entries(const ade_table_entry* left, const ade_table_entry* right) {
	if (left->Instanced == right->Instanced) {
		// Same type -> compare as normal
		return sort_table_entries(left, right);
	}
	if (left->Instanced) {
		return true;
	}
	return false;
}

void script_state::OutputLuaDocumentation(ScriptingDocumentation& doc,
	const scripting::DocumentationErrorReporter& errorReporter)
{
	SCP_vector<ade_table_entry*> table_entries;

	//***Everything
	for (uint32_t i = 0; i < ade_manager::getInstance()->getNumEntries(); i++) {
		auto ate = &ade_manager::getInstance()->getEntry(i);
		if (ate->ParentIdx == UINT_MAX)
			table_entries.push_back(ate);
	}

	std::sort(std::begin(table_entries), std::end(table_entries), sort_doc_entries);
	for (auto entry : table_entries) {
		doc.elements.emplace_back(entry->ToDocumentationElement(errorReporter));
	}

	//***Enumerations
	for (uint32_t i = 0; i < Num_enumerations; i++) {
		DocumentationEnum e;
		e.name = Enumerations[i].name;
		e.value = Enumerations[i].def;

		doc.enumerations.push_back(e);
	}

	auto& optionsList = options::OptionsManager::instance()->getOptions();
	for (auto& option : optionsList) {
		const options::OptionBase* thisOpt = option.get();

		DocumentationOption o;
		o.title = thisOpt->getTitle();
		o.description = thisOpt->getDescription();
		o.key = thisOpt->getConfigKey();

		doc.options.push_back(o);
	}
}

script_state* script_state::GetScriptState(lua_State* L)
{
	lua_getfield(L, LUA_REGISTRYINDEX, ScriptStateReferenceName);
	Assertion(lua_islightuserdata(L, -1), "Function called for Lua state that is not properly set up!");
	auto scriptStatePtr = lua_touserdata(L, -1);
	lua_pop(L, 1);

	return static_cast<script_state*>(scriptStatePtr);
}
