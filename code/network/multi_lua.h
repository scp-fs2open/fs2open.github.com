#pragma once

#include "network/multi.h"

#include <exception>

enum class lua_net_mode : uint8_t { RELIABLE, ORDERED, UNRELIABLE };

void process_lua_packet(ubyte* data, header* hinfo);
bool send_lua_packet(const luacpp::LuaValue& value, ushort target, lua_net_mode mode, net_player* reciever);