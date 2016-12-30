
#include "scripting/scripting.h"

/**
 * IMPORTANT!
 *
 * For various compiler reasons, all scripting API classes and libraries need to be referenced here! The easiest way to
 * do this is to declare the API elements in the header with the DECLARE_* macros and then include them here
 */

#include "scripting/api/bitops.h"
#include "scripting/api/enums.h"
#include "scripting/api/vecmath.h"
#include "scripting/api/event.h"
#include "scripting/api/file.h"
#include "scripting/api/font.h"
#include "scripting/api/gameevent.h"
#include "scripting/api/gamestate.h"
#include "scripting/api/hudgauge.h"
#include "scripting/api/eye.h"
#include "scripting/api/model.h"
#include "scripting/api/physics_info.h"
#include "scripting/api/sexpvar.h"
#include "scripting/api/shields.h"
#include "scripting/api/shiptype.h"
#include "scripting/api/species.h"
#include "scripting/api/team.h"
#include "scripting/api/streaminganim.h"
#include "scripting/api/texture.h"
#include "scripting/api/texturemap.h"
#include "scripting/api/weaponclass.h"
#include "scripting/api/mc_info.h"
#include "scripting/api/object.h"
#include "scripting/api/asteroid.h"
#include "scripting/api/cockpit_display.h"
#include "scripting/api/shipclass.h"
#include "scripting/api/debris.h"
#include "scripting/api/waypoint.h"
#include "scripting/api/ship_bank.h"
#include "scripting/api/subsystem.h"
#include "scripting/api/order.h"
#include "scripting/api/ship.h"
#include "scripting/api/sound.h"
#include "scripting/api/message.h"
#include "scripting/api/wing.h"
#include "scripting/api/beam.h"
#include "scripting/api/player.h"
#include "scripting/api/camera.h"
#include "scripting/api/control_info.h"
#include "scripting/api/particle.h"
#include "scripting/api/audio.h"
#include "scripting/api/base.h"
#include "scripting/api/cfile.h"
#include "scripting/api/weapon.h"
#include "scripting/api/controls.h"
#include "scripting/api/graphics.h"
#include "scripting/api/hud.h"
#include "scripting/api/hookvars.h"
#include "scripting/api/mission.h"
#include "scripting/api/tables.h"
#include "scripting/api/testing.h"

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

//	(void)l_BitOps.GetName();

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
	std::transform(std::begin(leftStr), std::end(leftStr), std::begin(leftStr), ::tolower);

	SCP_string rightStr(rightCmp);
	std::transform(std::begin(rightStr), std::end(rightStr), std::begin(rightStr), ::tolower);

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
