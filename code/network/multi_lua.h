#pragma once

#include "network/multi.h"
#include "scripting/api/objs/rpc.h"

#include <exception>

enum class lua_net_mode : uint8_t { RELIABLE, ORDERED, UNRELIABLE };
enum class lua_net_reciever : uint8_t { CLIENTS, SERVER, BOTH };

void process_lua_packet(ubyte* data, header* hinfo, bool reliable);
bool send_lua_packet(const luacpp::LuaValue& value, ushort target, lua_net_mode mode, lua_net_reciever reciever);

bool add_rpc(scripting::api::rpc_h_ref ref);
void clean_rpc_refs();