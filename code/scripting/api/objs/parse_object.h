#pragma once

#include "mission/missionparse.h"
#include "scripting/ade_api.h"

namespace scripting {
namespace api {

class parse_object_h {
	p_object* _obj;

  public:
	explicit parse_object_h(p_object* obj);

	p_object* getObject() const;

	bool isValid() const;

	void serialize(lua_State* /*L*/, const scripting::ade_table_entry& /*tableEntry*/, const luacpp::LuaValue& value, ubyte* data, int& packet_size);
	void deserialize(lua_State* /*L*/, const scripting::ade_table_entry& /*tableEntry*/, char* data_ptr, ubyte* data, int& offset);
};

DECLARE_ADE_OBJ(l_ParseObject, parse_object_h);

class parse_subsys_h {
	p_object* _obj = nullptr;
	int _subsys_offset = -1;

  public:
	parse_subsys_h();
	explicit parse_subsys_h(p_object* obj, int subsys_offset);

	subsys_status* getSubsys() const;

	bool isValid() const;

	void serialize(lua_State* /*L*/, const scripting::ade_table_entry& /*tableEntry*/, const luacpp::LuaValue& value, ubyte* data, int& packet_size);
	void deserialize(lua_State* /*L*/, const scripting::ade_table_entry& /*tableEntry*/, char* data_ptr, ubyte* data, int& offset);
};

DECLARE_ADE_OBJ(l_ParseSubsystem, parse_subsys_h);

} // namespace api
} // namespace scripting
