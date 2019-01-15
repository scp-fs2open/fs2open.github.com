
#include "scripting/scripting.h"

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
#include "scripting/api/objs/controls.h"
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
#include "scripting/api/objs/order.h"
#include "scripting/api/objs/particle.h"
#include "scripting/api/objs/physics_info.h"
#include "scripting/api/objs/player.h"
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
#include "scripting/api/libs/base.h"
#include "scripting/api/libs/bitops.h"
#include "scripting/api/libs/cfile.h"
#include "scripting/api/libs/graphics.h"
#include "scripting/api/libs/hookvars.h"
#include "scripting/api/libs/hud.h"
#include "scripting/api/libs/mission.h"
#include "scripting/api/libs/parse.h"
#include "scripting/api/libs/tables.h"
#include "scripting/api/libs/testing.h"
#include "scripting/api/libs/time_lib.h"
#include "scripting/api/libs/ui.h"
#include "scripting/api/libs/utf8.h"

// End of definitions includes

using namespace scripting;
using namespace scripting::api;

// *************************Housekeeping*************************
//WMC - The miraculous lines of code that make Lua debugging worth something.
lua_Debug Ade_debug_info;
char debug_stack[4][32];

void ade_debug_ret(lua_State *L, lua_Debug *ar)
{
	Assert(L != NULL);
	Assert(ar != NULL);
	lua_getstack(L, 1, ar);
	lua_getinfo(L, "nSlu", ar);
	memcpy(&Ade_debug_info, ar, sizeof(lua_Debug));

	int n;
	for (n = 0; n < 4; n++) {
		debug_stack[n][0] = '\0';
	}

	for (n = 0; n < 4; n++) {
		if (lua_getstack(L,n+1, ar) == 0)
			break;
		lua_getinfo(L,"n", ar);
		if (ar->name == NULL)
			break;
		strcpy_s(debug_stack[n],ar->name);
	}
}

//WMC - because the behavior of the return keyword
//was changed, I now have to use this in hooks.
static int ade_return_hack(lua_State *L)
{
	int i = 0;
	int num = lua_gettop(L);
	for(i = 0; i < num; i++)
	{
		lua_pushvalue(L, i+1);
	}

	return num;
}

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

	//*****SET DEBUG HOOKS
#ifndef NDEBUG
	lua_sethook(L, ade_debug_ret, LUA_MASKRET, 0);
#endif

	//*****INITIALIZE ADE
	uint i;
	mprintf(("LUA: Beginning ADE initialization\n"));
	for(i = 0; i < ade_manager::getInstance()->getNumEntries(); i++)
	{
		//WMC - Do only toplevel table entries, doi
		if(ade_manager::getInstance()->getEntry(i).ParentIdx == UINT_MAX)			//WMC - oh hey, we're done with the meaty point in < 10 lines.
			ade_manager::getInstance()->getEntry(i).SetTable(L, LUA_GLOBALSINDEX, LUA_GLOBALSINDEX);	//Oh the miracles of OOP.
	}

	//*****INITIALIZE RETURN HACK FUNCTION
	lua_pushstring(L, "ade_return_hack");
	lua_pushboolean(L, 0);
	lua_pushcclosure(L, ade_return_hack, 2);
	lua_setglobal(L, "ade_return_hack");

	//*****INITIALIZE ENUMERATION CONSTANTS
	mprintf(("ADE: Initializing enumeration constants...\n"));
	enum_h eh;
	for(i = 0; i < Num_enumerations; i++)
	{
		eh.index = Enumerations[i].def;
		eh.is_constant = true;

		ade_set_args(L, "o", l_Enum.Set(eh));
		lua_setglobal(L, Enumerations[i].name);
	}

	//*****ASSIGN LUA SESSION
	mprintf(("ADE: Assigning Lua session...\n"));
	SetLuaSession(L);

	//***** LOAD DEFAULT SCRIPTS
	mprintf(("ADE: Loading default scripts...\n"));
	load_default_script(L, "cfile_require.lua");

	return 1;
}

void script_state::EndLuaFrame()
{
	scripting::api::graphics_on_frame();
}

void ade_output_toc(FILE *fp, ade_table_entry *ate)
{
	Assert(fp != NULL);
	Assert(ate != NULL);

	//WMC - sanity checking
	if(ate->Name == NULL && ate->ShortName == NULL) {
		Warning(LOCATION, "Found ade_table_entry with no name or shortname");
		return;
	}

	fputs("<dd>", fp);

	if(ate->Name == NULL)
	{
		fprintf(fp, "<a href=\"#%s\">%s", ate->ShortName, ate->ShortName);
	}
	else
	{
		fprintf(fp, "<a href=\"#%s\">%s", ate->Name, ate->Name);
		if(ate->ShortName)
			fprintf(fp, " (%s)", ate->ShortName);
	}
	fputs("</a>", fp);

	if(ate->Description)
		fprintf(fp, " - %s\n", ate->Description);

	fputs("</dd>\n", fp);
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

	SCP_string leftStr(leftCmp);
	std::transform(std::begin(leftStr), std::end(leftStr), std::begin(leftStr),
	               [](char c) { return (char)::tolower(c); });

	SCP_string rightStr(rightCmp);
	std::transform(std::begin(rightStr), std::end(rightStr), std::begin(rightStr),
	               [](char c) { return (char)::tolower(c); });

	return leftStr < rightStr;
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

void script_state::OutputLuaMeta(FILE *fp)
{
	uint i;
	ade_table_entry *ate;
	fputs("<dl>\n", fp);

	//***Version info
	fprintf(fp, "<dd>Version: %s</dd>\n", LUA_RELEASE);

	SCP_vector<ade_table_entry*> table_entries;

	//***TOC: Libraries
	fputs("<dt><b>Libraries</b></dt>\n", fp);
	for(i = 0; i < ade_manager::getInstance()->getNumEntries(); i++)
	{
		ate = &ade_manager::getInstance()->getEntry(i);
		if(ate->ParentIdx == UINT_MAX && ate->Type == 'o' && ate->Instanced) {
			table_entries.push_back(ate);
		}
	}
	std::sort(std::begin(table_entries), std::end(table_entries), sort_table_entries);
	for (auto entry : table_entries) {
		ade_output_toc(fp, entry);
	}
	table_entries.clear();

	//***TOC: Objects
	fputs("<dt><b>Types</b></dt>\n", fp);
	for(i = 0; i < ade_manager::getInstance()->getNumEntries(); i++)
	{
		ate = &ade_manager::getInstance()->getEntry(i);
		if(ate->ParentIdx == UINT_MAX && ate->Type == 'o' && !ate->Instanced) {
			table_entries.push_back(ate);
		}
	}
	std::sort(std::begin(table_entries), std::end(table_entries), sort_table_entries);
	for (auto entry : table_entries) {
		ade_output_toc(fp, entry);
	}
	table_entries.clear();

	//***TOC: Enumerations
	fputs("<dt><b><a href=\"#Enumerations\">Enumerations</a></b></dt>", fp);

	//***End TOC
	fputs("</dl><br/><br/>", fp);

	//***Everything
	fputs("<dl>\n", fp);
	for(i = 0; i < ade_manager::getInstance()->getNumEntries(); i++)
	{
		ate = &ade_manager::getInstance()->getEntry(i);
		if(ate->ParentIdx == UINT_MAX)
			table_entries.push_back(ate);
	}

	std::sort(std::begin(table_entries), std::end(table_entries), sort_doc_entries);
	for (auto entry : table_entries) {
		entry->OutputMeta(fp);
	}
	table_entries.clear();

	//***Enumerations
	fprintf(fp, "<dt id=\"Enumerations\"><h2>Enumerations</h2></dt>");
	for(i = 0; i < Num_enumerations; i++)
	{
		//WMC - This is in case we ever want to add descriptions to enums.
		//fprintf(fp, "<dd><dl><dt><b>%s</b></dt><dd>%s</dd></dl></dd>", Enumerations[i].name, Enumerations[i].desc);

		//WMC - Otherwise, just use this.
		fprintf(fp, "<dd><b>%s</b></dd>", Enumerations[i].name);
	}
	fputs("</dl>\n", fp);

	//***End LUA
	fputs("</dl>\n", fp);
}
