#pragma once
#ifndef FS2_OPEN_GAMESTATE_H
#define FS2_OPEN_GAMESTATE_H

#include "scripting/ade_api.h"
#include "gamesequence/gamesequence.h"

namespace scripting {
namespace api {

class gamestate_h {
 private:
	int sdx;
 public:
	gamestate_h();
	gamestate_h(int n_state);

	bool IsValid();

	int Get();

	void serialize(lua_State* /*L*/, const scripting::ade_table_entry& /*tableEntry*/, const luacpp::LuaValue& value, ubyte* data, int& packet_size);
	void deserialize(lua_State* /*L*/, const scripting::ade_table_entry& /*tableEntry*/, char* data_ptr, ubyte* data, int& offset);
};

DECLARE_ADE_OBJ(l_GameState, gamestate_h);


}
}

#endif // FS2_OPEN_GAMESTATE_H
