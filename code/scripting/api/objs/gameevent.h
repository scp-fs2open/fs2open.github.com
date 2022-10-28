#pragma once
#ifndef FS2_OPEN_GAMEEVENT_H
#define FS2_OPEN_GAMEEVENT_H

#include "scripting/ade_api.h"
#include "gamesequence/gamesequence.h"

namespace scripting {
namespace api {

class gameevent_h
{
private:
	int edx;
public:
	gameevent_h();

	explicit gameevent_h(int n_event);

	bool IsValid();

	int Get();

	void serialize(lua_State* /*L*/, const scripting::ade_table_entry& /*tableEntry*/, const luacpp::LuaValue& value, ubyte* data, int& packet_size);
	void deserialize(lua_State* /*L*/, const scripting::ade_table_entry& /*tableEntry*/, char* data_ptr, ubyte* data, int& offset);
};

DECLARE_ADE_OBJ(l_GameEvent, gameevent_h);

}
}


#endif
