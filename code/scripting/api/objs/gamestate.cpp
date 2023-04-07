//
//

#include "gamestate.h"

#include "network/multi.h"
#include "network/multimsgs.h"
#include "network/multiutil.h"

namespace scripting {
namespace api {

gamestate_h::gamestate_h() { sdx = -1; }

gamestate_h::gamestate_h(int n_state) { sdx = n_state; }

bool gamestate_h::IsValid() { return (sdx > -1 && sdx < Num_gs_state_text); }

int gamestate_h::Get() { return sdx; }

void gamestate_h::serialize(lua_State* /*L*/, const scripting::ade_table_entry& /*tableEntry*/, const luacpp::LuaValue& value, ubyte* data, int& packet_size) {
	gamestate_h event;
	value.getValue(l_GameState.Get(&event));
	ADD_INT(event.sdx);
}

void gamestate_h::deserialize(lua_State* /*L*/, const scripting::ade_table_entry& /*tableEntry*/, char* data_ptr, ubyte* data, int& offset) {
	int index;
	GET_INT(index);
	new(data_ptr) gamestate_h(index);
}

ADE_OBJ(l_GameState, gamestate_h, "gamestate", "Game state");

ADE_FUNC(__tostring, l_GameState, NULL, "Game state name", "string", "Game state name, or empty string if handle is invalid")
{
	gamestate_h *gh = NULL;
	if(!ade_get_args(L, "o", l_GameState.GetPtr(&gh)))
		return ade_set_error(L, "s", "");

	if(!gh->IsValid())
		return ade_set_error(L, "s", "");

	return ade_set_args(L, "s", GS_state_text[gh->Get()]);
}

ADE_VIRTVAR(Name, l_GameState,"string", "Game state name", "string", "Game state name, or empty string if handle is invalid")
{
	gamestate_h *gh = NULL;
	const char* n_name = nullptr;
	if(!ade_get_args(L, "o|s", l_GameState.GetPtr(&gh), &n_name))
		return ade_set_error(L, "s", "");

	if(!gh->IsValid())
		return ade_set_error(L, "s", "");

	int sdx = gh->Get();

	if(ADE_SETTING_VAR)
	{
		Error(LOCATION, "Can't set game state names at this time");
	}

	return ade_set_args(L, "s", GS_state_text[sdx]);
}


}
}
